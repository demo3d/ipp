#!/usr/bin/env python3


import os
import os.path as path
import click
from subprocess import call


root_dir = path.dirname(path.realpath(__file__))


@click.command()
@click.option('--clang-path', envvar='CLANG', help='Path to clang install')
def clang_format(clang_path):
    clang_format = path.join(clang_path, 'bin/clang-format')

    source_dirs = [
        'ipp/lib',
        'ipp/capi',
        'ipp/devshell']

    for input_dir in source_dirs:
        input_dir = path.join(root_dir, input_dir)
        for root, dirs, files in os.walk(input_dir):
            for source in files:
                if source.endswith('.cpp') or source.endswith('.hpp'):
                    source_file = path.join(root, source)
                    call('{} -i -style=file {}'.format(clang_format, source_file), shell=True)


if __name__ == '__main__':
    clang_format()
