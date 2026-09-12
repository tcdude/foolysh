"""
python-for-android build recipe for foolysh.
"""

from multiprocessing import cpu_count

from pythonforandroid.recipe import CppCompiledComponentsPythonRecipe


class FoolyshRecipe(CppCompiledComponentsPythonRecipe):
    version = 'master'  # The branch or tag to use
    url = 'https://github.com/tcdude/foolysh/archive/{version}.zip'
    site_packages_name = 'foolysh'
    call_hostpython_via_targetpython = False
    need_stl_shared = True
    hostpython_prerequisites = ['cython>=3.1']

    depends = ['python3', 'numpy', 'pysdl2', 'Pillow', 'plyer', 'setuptools']

    def build_compiled_components(self, arch):
        self.setup_extra_args = ['-j', str(cpu_count())]
        super().build_compiled_components(arch)
        self.setup_extra_args = []

    def rebuild_compiled_components(self, arch, env):
        self.setup_extra_args = ['-j', str(cpu_count())]
        super().rebuild_compiled_components(arch, env)
        self.setup_extra_args = []


recipe = FoolyshRecipe()
