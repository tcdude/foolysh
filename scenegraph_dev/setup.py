from distutils.core import setup

from Cython.Build import cythonize

setup(
    ext_modules=cythonize(
        ['aabb.pyx', 'vector2.pyx', 'nodepath.pyx'],
        compiler_directives={'language_level': 3},
    )
)
