"""Setup for foolysh."""

import os
import sys
import glob
import platform
from setuptools import setup
from setuptools import Extension
from setuptools import find_namespace_packages

try:
    from Cython.Build import cythonize
    USE_CYTHON = True
except ImportError:
    USE_CYTHON = False

__author__ = 'Tiziano Bettio'
__license__ = 'MIT'
__version__ = '0.1'
__copyright__ = """
Copyright (c) 2020 Tiziano Bettio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

if '--no-cython' in sys.argv:
    USE_CYTHON = False
    sys.argv.remove('--no-cython')

ROOT = os.path.dirname(os.path.abspath(__file__))

with open(os.path.join(ROOT, 'VERSION'), 'r', encoding='utf-8') as f:
    VERSION = f.read().strip()


EXTRA_COMPILE_ARGS = []
EXTRA_LINK_ARGS = []
LIBRARIES = []

if platform.system() == 'Linux':
    EXTRA_COMPILE_ARGS.append('-std=c++11')
    EXTRA_LINK_ARGS.append('-std=c++11')
elif platform.system() == 'Darwin':
    EXTRA_COMPILE_ARGS.append('-std=c++11')
    EXTRA_LINK_ARGS.append('-stdlib=libc++')
    EXTRA_LINK_ARGS.append('-mmacosx-version-min=10.9')

if 'ARCH' in os.environ and os.environ['ARCH'].startswith('arm'):
    EXTRA_COMPILE_ARGS.append('-fexceptions')
    EXTRA_LINK_ARGS.append('-fexceptions')
    LIBRARIES.append('c++_shared')

EXT = '.pyx' if USE_CYTHON else '.cpp'
PATTERN = os.path.join('src', 'foolysh', '**', '*' + EXT)


def _module_name(path):
    """Turn a source path under src/ into a dotted module name."""
    rel = os.path.relpath(path, 'src').replace('\\', '/')
    if rel.endswith(EXT):
        rel = rel[:-len(EXT)]
    return rel.replace('/', '.')


EXTENSION = [
    Extension(
        _module_name(i),
        [i.replace('\\', '/')],
        include_dirs=[os.path.join(ROOT, 'ext')],
        extra_compile_args=EXTRA_COMPILE_ARGS,
        extra_link_args=EXTRA_LINK_ARGS,
        language='c++',
        libraries=LIBRARIES,
    )
    for i in glob.glob(PATTERN, recursive=True)
]


def ext_modules():
    """Optionally cythonize."""
    if USE_CYTHON:
        return cythonize(
            EXTENSION,
            compiler_directives={
                'language_level': 3,
                'embedsignature': True,
            },
            annotate=False,
        )
    return EXTENSION


setup(
    name='foolysh',
    version=VERSION,
    description='A 2D Rendering Engine, nobody asked for or needed.',
    author='Tiziano Bettio',
    author_email='tizilogic@gmail.com',
    python_requires='>=3.9',
    packages=find_namespace_packages(where='src'),
    package_dir={'': 'src'},
    package_data={'foolysh': ['assets/*.png']},
    include_package_data=True,
    zip_safe=False,
    install_requires=[
        'plyer>=2.1.0',
        'Pillow>=9.1.0',
        'PySDL2>=0.9.16',
        'numpy>=2.2.0',
    ],
    ext_modules=ext_modules(),
)
