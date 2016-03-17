#!/usr/bin/env python3


import os
import os.path as path
import shutil
import click
from subprocess import call


root_dir = path.dirname(path.realpath(__file__))


@click.group()
def cli():
    pass


@cli.command()
@click.option('--flatc', default="flatc", envvar='FLATC', help='FlatBuffers compiler executable')
def schema(flatc):
    """
    Compile schema sources in to schema folder :

        python modules are compiled under build/schema/py/
        c++ headers are compiled under build/schema/cpp/
        javascript modules are compiled under build/schema/js/
    """
    src_dir = path.join(root_dir, 'schema')
    schema_dest_dir = path.join(root_dir, 'build/schema')
    cpp_dest_dir = path.join(schema_dest_dir, 'cpp')
    py_dest_dir = path.join(schema_dest_dir, 'py')
    js_dest_dir = path.join(schema_dest_dir, 'js')

    shutil.rmtree(cpp_dest_dir, ignore_errors=True)
    os.makedirs(cpp_dest_dir, exist_ok=True)

    shutil.rmtree(py_dest_dir, ignore_errors=True)
    os.makedirs(py_dest_dir, exist_ok=True)

    shutil.rmtree(js_dest_dir, ignore_errors=True)
    os.makedirs(js_dest_dir, exist_ok=True)

    for root, dirs, files in os.walk(src_dir):
        root_rel = path.relpath(root, src_dir)
        for src in files:
            src_path = path.join(root, src)
            if src.endswith('.fbs'):
                # compile cpp header
                cpp_out_dir = path.join(cpp_dest_dir, root_rel)
                os.makedirs(cpp_out_dir, exist_ok=True)
                cpp_compile = '{flatc} -c -I "{src_root}" --no-includes -o "{out_dir}" "{src}"'
                cpp_compile = cpp_compile.format(
                        flatc=flatc,
                        src_root=src_dir.replace('\\', '/'),
                        out_dir=cpp_out_dir.replace('\\', '/'),
                        src=src_path.replace('\\', '/'))
                call(cpp_compile, shell=True)

                # compile python module
                py_compile = '{flatc} -p -I "{src_root}" -o "{out_dir}" "{src}"'
                py_compile = py_compile.format(
                        flatc=flatc,
                        src_root=src_dir.replace('\\', '/'),
                        out_dir=py_dest_dir.replace('\\', '/'),
                        src=src_path.replace('\\', '/'))
                call(py_compile, shell=True)

                # compile js module
                js_out_dir = path.join(js_dest_dir, root_rel)
                os.makedirs(js_out_dir, exist_ok=True)
                js_compile = '{flatc} -s -I "{src_root}" -o "{out_dir}" "{src}"'
                js_compile = js_compile.format(
                        flatc=flatc,
                        src_root=src_dir.replace('\\', '/'),
                        out_dir=js_out_dir.replace('\\', '/'),
                        src=src_path.replace('\\', '/'))
                call(js_compile, shell=True)

        for src in files:
            if src.endswith('.py'):
                dest_dir = path.join(py_dest_dir, root_rel)
                dest_file = path.join(dest_dir, src)
                os.makedirs(dest_dir, exist_ok=True)
                shutil.copy(src_path, dest_file)


@cli.command()
@click.option('-o', '--output-dir', default=None, help='CMake output directory root, default build/emscripten/(release|debug)')
@click.option('--debug/--release', default=True, help='CMAKE_BUILD_TYPE Debug or Release')
@click.option('--logging-enabled/--logging-disabled', default=True, help='Configures CMake option for IVL_LOGGING_DISABLED')
@click.option('--emscripten-path', envvar='EMSCRIPTEN', prompt=True, help='Path to Emscripten root')
@click.option('--print-commands/--run', default=False, help='Only print configure commands, don\'t execute them')
def emscripten(output_dir, debug, logging_enabled, emscripten_path, print_commands):
    emscripten_toolchain = path.join(emscripten_path, 'cmake/Modules/Platform/Emscripten.cmake')

    if output_dir == None:
        output_dir = path.join(root_dir, 'build/emscripten', 'debug' if debug else 'release')

    cmake_command = 'cmake -G"Ninja" -H"{}" -B"{}" -DCMAKE_BUILD_TYPE={} -DIVL_LOGGING_DISABLED={} -DCMAKE_TOOLCHAIN_FILE={}'.format(
            path.join(root_dir, 'ipp'),
            output_dir,
            'Debug' if debug else 'Release',
            'OFF' if logging_enabled else 'ON',
            emscripten_toolchain)

    if print_commands:
        print(cmake_command)
    else:
        call(cmake_command, shell=True)


@cli.command()
@click.option('-o', '--output-dir', default=None, help='CMake output directory root, default build/linux/(release|debug)')
@click.option('--debug/--release', default=True, help='CMAKE_BUILD_TYPE Debug or Release')
@click.option('--clang-path', envvar='CLANG', help='Path to clang install')
@click.option('--print-commands/--run', default=False, help='Only print configure commands, don\'t execute them')
def linux(output_dir, debug, clang_path, print_commands):
    if output_dir == None:
        output_dir = path.join(root_dir, 'build/linux', 'debug' if debug else 'release')

    cmake_command = 'cmake -H"{}" -B"{}" -DCMAKE_C_COMPILER="{}"  -DCMAKE_CXX_COMPILER="{}" -DCMAKE_BUILD_TYPE={}'.format(
        path.join(root_dir, 'ipp'), output_dir,
        path.join(clang_path, 'bin/clang') if clang_path else 'clang',
        path.join(clang_path, 'bin/clang++') if clang_path else 'clang++',
        'Debug' if debug else 'Release')

    if print_commands:
        print(cmake_command)
    else:
        call(cmake_command, shell=True)


@cli.command()
def clean():
    shutil.rmtree(path.join(root_dir, 'build'), ignore_errors=True)


if __name__ == '__main__':
    cli()
