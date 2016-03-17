import os
import os.path as path
import struct


class ResourceDefinition:
    """
    Defines a resource that is registered with a Package object (see Package.register_resource)
    """
    def __init__(self, resource_path, kind):
        self._kind = kind
        self._resource_path = resource_path

    @property
    def kind(self):
        """
        Resource kind identifier string
        """
        return self._kind

    @property
    def resource_path(self):
        """
        Resource output file path (relative to parent Package output path)
        """
        return self._resource_path


class Package:
    """
    Package is defined by name and root input/output paths.
    Package object keeps track of all files registered to the package as members and
    provides methods to archive them.
    """
    def __init__(self, package_name, input_path, output_path):
        self._package_name = package_name
        self._input_path = input_path
        self._output_path = output_path
        self._resources = set()

    @property
    def resources(self):
        """
        Set of output resource ResourceDefinition objects registered with this package.
        """
        return self._resources

    @property
    def package_name(self):
        """
        Package unique name.
        """
        return self._package_name

    @property
    def input_path(self):
        """
        Absolute directory path from where package input files are read.
        """
        return self._input_path

    @property
    def output_path(self):
        """
        Absolute directory path to where package output files are written.
        """
        return self._output_path

    def resource_output_file_path(self, resource_path):
        """
        Return resource file path absolute in package output directory.
        """
        if ':' in resource_path:
            sep_index = resource_path.find(':')
            assert resource_path[:sep_index] == self._package_name
            resource_path = resource_path[sep_index + 1:]
        assert not path.isabs(resource_path), resource_path
        return path.join(self._output_path, resource_path)

    def create_resource_output_file_path(self, resource_path):
        """
        Create path leading up to resource_path in output directory if it doesn't exist.
        resource_path can be absolute file path or a resource_path.
        """
        if path.isabs(resource_path):
            resource_output_path = resource_path
        else:
            resource_output_path = self.resource_output_file_path(resource_path)
        os.makedirs(path.dirname(resource_output_path), exist_ok=True)
        return resource_output_path

    def resource_path_from_source(self, source_path, ext=None):
        """
        Returns relative package resource path for source path (must be inside input path).
        If ext is not none it replaces source ext.
        """
        if path.isabs(source_path):
            assert path.commonprefix([self._input_path, source_path]) == self._input_path
            source_path = path.relpath(source_path, self._input_path)
        if ext is not None:
            source_path = path.splitext(source_path)[0] + ext
        return source_path

    def create(self, resource_path, *args, **kwargs):
        """
        Open a resource file at relative resource_path (missing subfolders are created).
        Rest of the arguments are forwarded to python open function.
        """
        resource_path_abs = self.create_resource_output_file_path(resource_path)
        file_handle = open(resource_path_abs, *args, **kwargs)
        return file_handle

    def register_resource(self, resource_path, kind):
        """
        Register resource output file path (absolute or package output relative) and kind with package.

        Registered resource will be created after pipeline build tasks complete and can be then packaged
        in to archives or further processed/concatenated.
        """
        if path.isabs(resource_path):
            resource_path = path.relpath(resource_path, self.output_path)
        self._resources.add(ResourceDefinition(resource_path, kind))

    def archive_ipp(self, archive_path, resource_filter):
        """
        Create a ipp custom format archive file at archive_path with all resources archived in it.
        resource_filter is a function that takes ResourceDefinition instance and returns True if the
        resource should be added to archive

        ipp archive is structured as binary file :
            uint32 - size of header
            header - flatbuffer Archive table defined in ipp.schema.resource.archive
            data   - archive data described in header

        Data can be optionally compressed.
        ToDo: Implement compression
        """
        import flatbuffers
        from ipp.schema.resource.archive import Archive, File

        resource_file_paths = [path.join(self.output_path, resource.resource_path)
                               for resource in self.resources if resource_filter(resource)]

        builder = flatbuffers.Builder(1024 * 128)
        file_offsets = []
        current_file_offset = 0
        for resource_file_path in resource_file_paths:
            file_stat = os.stat(resource_file_path)
            resource_file_path_rel = path.relpath(resource_file_path, self.output_path)
            name_offset = builder.CreateString(resource_file_path_rel.replace('\\', '/'))

            File.FileStart(builder)
            File.FileAddName(builder, name_offset)
            File.FileAddSize(builder, file_stat.st_size)
            file_offsets.append(File.FileEnd(builder))

            current_file_offset += file_stat.st_size

        Archive.ArchiveStartFilesVector(builder, len(file_offsets))
        for file_offset in reversed(file_offsets):
            builder.PrependUOffsetTRelative(file_offset)
        files_offset = builder.EndVector(len(file_offsets))

        Archive.ArchiveStart(builder)
        Archive.ArchiveAddFiles(builder, files_offset)
        builder.Finish(Archive.ArchiveEnd(builder))

        with open(archive_path, 'wb') as ipp_archive:
            header = builder.Output()
            ipp_archive.write(struct.pack('I', len(header)))
            ipp_archive.write(header)
            for resource in resource_file_paths:
                with open(resource, 'rb') as resource_file:
                    resource_file_data = resource_file.read()
                    ipp_archive.write(resource_file_data)
