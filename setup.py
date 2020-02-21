"""Setup for foolysh."""

import os
import platform
from setuptools import setup
from setuptools import find_namespace_packages
from distutils.extension import Extension  # pylint: disable=wrong-import-order
from Cython.Build import cythonize

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


with open(os.path.join(os.path.dirname(__file__), 'VERSION'), 'r') as f:
    VERSION = f.read().strip()


EXTRA_COMPILE_ARGS = []
EXTRA_LINK_ARGS = []
if platform.system() == 'Linux':
    EXTRA_COMPILE_ARGS.append('-std=c++11')
    EXTRA_LINK_ARGS.append('-std=c++11')


EXTENSION = [
    Extension(
        '*',
        ['src/foolysh/**/*.pyx'],
        include_dirs=['ext'],
        extra_compile_args=EXTRA_COMPILE_ARGS,
        extra_link_args=EXTRA_LINK_ARGS
    )
]


setup(
    name='foolysh',
    version=VERSION,
    description='A 2D Rendering Engine, nobody asked for or needed.',
    author='Tiziano Bettio',
    author_email='tizilogic@gmail.com',
    packages=find_namespace_packages(where='src'),
    package_dir={'': 'src'},
    package_data={'foolysh': [
        'LICENSE.md',
    ]},
    install_requires=['plyer', 'Pillow', 'PySDL2>=0.9.6', 'numpy>=1.18'],
    setup_requires=['Cython'],
    ext_modules=cythonize(
        EXTENSION,
        compiler_directives={'language_level': 3, 'embedsignature': True},
        annotate=False),
)
