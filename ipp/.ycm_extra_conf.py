import os
import os.path as path


root_dir = path.dirname(path.abspath(__file__))


def FlagsMakePathsAbsolute(flags, working_directory):
    if not working_directory:
        return list(flags)

    new_flags = []
    make_next_absolute = False
    path_flags = [ '-isystem', '-I', '-iquote', '--sysroot=' ]
    for flag in flags:
        new_flag = flag

        if make_next_absolute:
            make_next_absolute = False
            if not flag.startswith('/'):
                new_flag = path.join(working_directory, flag)
        else:
            for path_flag in path_flags:
                if flag == path_flag:
                    make_next_absolute = True
                    break
        new_flags.append(new_flag)
    return new_flags


default_compilation_flags = [
    '-Wall',
    '-Wextra',
    '-DNDEBUG',
    '-DIVL_DEBUG_BUILD',
    '-std=c++14',
    '-x', 'c++',
    '-isystem', '../build/schema/cpp',
    '-isystem', 'extern',
    '-isystem', 'extern/glm',
    '-isystem', 'extern/flatbuffers',
    '-isystem', 'extern/lodepng',
    '-isystem', 'extern/Catch/single_include'
]

if 'EMSCRIPTEN' in os.environ:
    default_compilation_flags.extend([
        '-isystem', path.join(os.environ['EMSCRIPTEN'], 'system/include'),
        '-isystem', path.join(os.environ['EMSCRIPTEN'], 'system/include/emscripten')])

project_flags = {}

project_flags['lib'] = ['-I', 'lib/include']
project_flags['capi'] = project_flags['lib']
project_flags['devshell'] = project_flags['lib'] + ['-I', 'devshell/include']


def FlagsForFile(filename, **kwargs):
    compilation_flags = default_compilation_flags[:]
    for root, flags in project_flags.items():
        if root in filename:
            compilation_flags.extend(flags)
            break

    return {
        'flags': FlagsMakePathsAbsolute(compilation_flags, root_dir),
        'do_cache': True}
