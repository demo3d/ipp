import os.path as path
import shutil
import yaml
import bpy


build_generators = {}


def open_blend(blend_path):
    if bpy.data.filepath != blend_path:
        bpy.ops.wm.open_mainfile(filepath=blend_path)


def build_generator(kind):
    """
    Decorator function that registers a build task generator function with resource kind string.

    Build task generator should accept the following arguments :
        * spec - ResourceSpec
        * resource_name - unique resource name (within spec) string
        * data - dict containing resource data

    It should yield pydoit task description dicts
    """
    def wrapper(generator_fn):
        if kind in build_generators:
            raise ValueError('Generator already registered for resource kind {}'.format(kind))
        build_generators[kind] = generator_fn
        return generator_fn
    return wrapper


def build_copy(package, resource_path, source_path):
    package.create_resource_output_file_path(resource_path)
    shutil.copy(source_path, resource_path)


@build_generator('copy')
def build_generator_copy(manifest, resource_name, data):
    """
    Copy task simply copy file from source path property to resource_name.
    """
    package = manifest.package
    register = data.get('register', True)
    source_path = manifest.source_path(data['source'])
    resource_path = manifest.qualified_resource_path(resource_name)
    resource_output = package.resource_output_file_path(resource_path)

    yield {'name': 'copy:' + resource_path,
           'actions': [(build_copy, [package, resource_output, source_path])],
           'file_dep': [source_path],
           'targets': [resource_output]}


class BuildManifest:
    """
    Build manifest generates resource build definitions from a build manifest YAML file and
    context resource package object/global variables.
    """
    def __init__(self, manifest_path, package_manifest):
        self._manifest_path = manifest_path
        self._package_manifest = package_manifest
        self._rel_dir = path.relpath(
            path.realpath(path.dirname(manifest_path)),
            self._package_manifest.package.input_path)
        if self._rel_dir == '.':
            self._rel_dir = ''
        self._resource_definitions = yaml.load(open(manifest_path, 'rt'))

    @property
    def package_manifest(self):
        """
        PackageManifest object this BuildManifest belongs to.
        """
        return self._package_manifest

    @property
    def defaults(self):
        """
        Default properties dict inherited from parent package manifest yaml file.
        """
        return self._package_manifest.defaults

    @property
    def package(self):
        """
        Parent Package object.
        """
        return self._package_manifest.package

    @property
    def manifest_path(self):
        """
        Resource manifest source YAML file absolute path.
        """
        return self._manifest_path

    @property
    def rel_dir(self):
        """
        Package relative path to manifest file package directory.
        Resource relative paths are joined to this path.
        """
        return self._rel_dir

    @property
    def resource_definitions(self):
        """
        Dict of resource name -> definition data for every resource defined in resource context file.
        """
        return self._resource_definitions

    def qualified_resource_path(self, resource_path):
        """
        Return fully qualified resource path from resource_path.

        If resource path is prefixed with package name (eg. 'package:foo/bar.baz')
        resource_path is returned unmodified.

        If resource path starts with '/' it is prefixed with package_definition path name.

        Otherwise resource path is prefixed with "package_name:context_rel_dir/"
        """
        resource_path = resource_path.replace('\\', '/')
        if ':' in resource_path:
            sep_index = resource_path.find(':')
            if resource_path[sep_index + 1] == '/':
                return resource_path[:sep_index + 1] + resource_path[sep_index + 2:]
            return resource_path

        if resource_path[0] == '/':
            return '{}:{}'.format(self.package.package_name, resource_path[1:])

        return '{}:{}'.format(self.package.package_name, path.join(self.rel_dir, resource_path).replace('\\', '/'))

    def source_path(self, source):
        """
        Return absolute source path from source path string.
        If string starts with '/' it' relative to package input root,
        otherwise it's relative to spec file parent dir.
        """
        source = source.replace('\\', '/')
        if source[0] == '/':
            return path.join(self.package.input_path, source[1:])
        return path.join(self.package.input_path, self.rel_dir, source)


class PackageGenerator:
    def __init__(self, package_manifest_path, input_prefix, output_prefix):
        from ..package import Package

        self._package_manifest_path = package_manifest_path
        with open(package_manifest_path) as package_manifest_file:
            package_manifest = yaml.load(package_manifest_file)
            self._build_manifests = package_manifest['build_manifests']
            self._defaults = package_manifest.get('defaults', {})

            package_name = package_manifest['name']
            package_input = path.join(input_prefix, package_manifest['input'])
            package_output = path.join(output_prefix, package_manifest['output'])
            self._package = Package(package_name, package_input, package_output)

    @property
    def package_manifest_path(self):
        """
        Absolute path to source package manifest YAML file.
        """
        return self._package_manifest_path

    @property
    def build_manifests(self):
        """
        List of file paths to build manifest YAML files included in package manifest.
        Paths are relative to package manifest YAML file.
        """
        return self._build_manifests

    @property
    def package(self):
        return self._package

    @property
    def defaults(self):
        return self._defaults

    def generate_build_tasks(self):
        """
        Yield build tasks from package_manifest YAML file.

        Package manifest file defines :
            * name - (unique) package name string
            * input - input directory (relative to input_prefix, ie. input root path)
            * output - output directory (relative to output_prefix, ie. output root path)
            * defaults - map of package level default configuration values for resource builders
            * manifests - list of relative paths to resource manifest YAML files

        Package targets will be compressed to the package_output parent folder with package_name
        """
        for manifest_path in self.build_manifests:
            manifest_path = path.join(path.dirname(self.package_manifest_path), manifest_path)
            manifest = BuildManifest(manifest_path, self)

            for resource_name, data in manifest.resource_definitions.items():
                kind = data['kind']
                generator = build_generators[kind]
                for task_dict in generator(manifest, resource_name, data):
                    # resource tasks implicitly depend on manifest definition file
                    if 'file_dep' not in task_dict:
                        task_dict['file_dep'] = []
                    task_dict['file_dep'].append(manifest.manifest_path)
                    # register task output resources with package
                    for resource_path in task_dict['targets']:
                        self.package.register_resource(resource_path, kind)
                    yield task_dict

    def generate_archive_ipp_task(self, archive_path, resource_filter=lambda _: True):
        """
        Return a package task that will pack all package resources to a ipp archive file at archive_path.
        """
        yield {
            'name': 'package-archive-ipp:' + archive_path,
            'targets': [archive_path],
            'file_dep': [path.join(self.package.output_path, resource.resource_path) for resource in self.package.resources],
            'actions': [(self.package.archive_ipp, (archive_path, resource_filter), {})]}
