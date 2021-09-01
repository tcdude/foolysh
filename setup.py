"""Setup for foolysh."""

import os
import sys
import glob
import platform
from setuptools import setup
from setuptools import find_namespace_packages
from distutils.extension import Extension  # pylint: disable=wrong-import-order
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

with open(os.path.join(os.path.dirname(__file__), 'VERSION'), 'r') as f:
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
EXTENSION = [
    Extension(
        i[4:-4].replace('/', '.').replace('\\', '.'),
        [i],
        include_dirs=['ext'],
        extra_compile_args=EXTRA_COMPILE_ARGS,
        extra_link_args=EXTRA_LINK_ARGS,
        language='c++',
        libraries=LIBRARIES
    )
    for i in glob.glob('src/foolysh/**/*' + EXT, recursive=True)
]


def ext_modules():
    """Optionally cythonize."""
    if USE_CYTHON:
        return cythonize(EXTENSION,
                         compiler_directives={'language_level': 3,
                                              'embedsignature': True},
                         annotate=False)
    return EXTENSION


setup(
    name='foolysh',
    version=VERSION,
    description='A 2D Rendering Engine, nobody asked for or needed.',
    author='Tiziano Bettio',
    author_email='tizilogic@gmail.com',
    packages=find_namespace_packages(where='src'),
    package_dir={'': 'src'},
    package_data={'': ['LICENSE.md',],
                  'foolysh': ['assets/*.png']},
    install_requires=['plyer', 'Pillow', 'PySDL2>=0.9.6', 'numpy>=1.18'],
    ext_modules=ext_modules(),
)
