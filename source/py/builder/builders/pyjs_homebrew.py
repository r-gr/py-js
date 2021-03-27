"""homebrew: using homebrew python project as make pyjs max externals


"""

import pathlib
import platform

from ..projects import Project
from .pyjs import PyJsBuilder


class HomebrewProject(Project):
    """Project to build Python from source with different variations."""
    root = pathlib.Path.cwd()
    scripts = root / 'scripts'
    targets = root / 'targets'
    build = targets / 'build'

    pyjs = root.parent.parent
    support = pyjs / 'support'
    externals = pyjs / 'externals'
    # frameworks = support / 'Frameworks'

    py_version = platform.python_version()
    py_ver = ".".join(py_version.split('.')[:2])
    py_name = f'python{py_ver}'
    name = py_name

    prefix = support / py_name
    bin = prefix / 'bin'
    lib = prefix / 'lib' / py_name
    dylib = f'libpython_{py_ver}.dylib'
    homebrew = f'/usr/local/opt/python3/Frameworks/Python.framework/Versions/{py_ver}'

    py_external = externals / 'py.mxo'
    pyjs_external = externals / 'pyjs.mxo'


class HomebrewBuilder(PyJsBuilder):
    """A Python Builder using Homebrew"""
    name = 'python'
    project_class = HomebrewProject
    version = platform.python_version()
    depends_on = []
    suffix = ""
    setup_local = None
    patch = None

    @property
    def prefix(self):
        """compiled product destination root directory."""
        return self.project.prefix

    def cp_pkgs(self, pkgs):
        for pkg in pkgs:
            # self.log("copying %s", pkg)
            self.cmd(f"cp -rf {self.project.homebrew}/lib/{self.project.name}/{pkg} {self.project.lib}/{pkg}")

    def rm_libs(self, names):
        """remove all named python dylib libraries"""
        for name in names:
            self.remove(self.python_lib / name)

    def remove_extensions(self):
        """remove extensions: not implemented"""

    def clean(self):
        """clean everything."""
        self.clean_python_pyc(self.prefix)
        self.clean_python_tests(self.python_lib)
        # self.clean_python_site_packages()

        for i in (self.python_lib / 'distutils' / 'command').glob('*.exe'):
            self.remove(i)

        # self.remove(self.prefix_lib / 'pkgconfig')
        # self.remove(self.prefix / 'share')

        self.remove_packages()
        self.remove_extensions()
        # self.remove_binaries()

    def fix_python_exec(self):
        self.chdir(self.project.bin)
        self.cmd(f'install_name_tool -change {self.project.homebrew}/Python'
                 f' @executable_path/../{self.dylib} {self.project.name}')
        self.chdir(self.project.root)

    def fix_python_dylib_for_pkg(self):
        self.chdir(self.project.prefix)
        self.chmod(self.dylib)
        # assumes python in installed in $PREFIX
        self.install_name_tool(
            f'@loader_path/../../../../support/{self.project.name}/{self.dylib}', self.dylib)
        self.chdir(self.project.root)

    def fix_python_dylib_for_ext_executable(self):
        self.chdir(self.project.prefix)
        self.chmod(self.dylib)
        # assumes cp -rf $PREFIX/* -> same directory as py extension in py.mxo
        self.install_name_tool(f'@loader_path/{self.dylib}', self.dylib)
        self.cmd(f'cp -rf {self.prefix}/* {self.project.py_external}/Contents/MacOS')
        self.chdir(self.project.root)

    def fix_python_dylib_for_ext_executable_name(self):
        self.chdir(self.prefix)
        self.chmod(self.dylib)
        self.install_name_tool(f'@loader_path/{self.dylib}', self.dylib)
        self.cmd(f'mkdir -p {self.project.py_external}/Contents/MacOS/{self.project.name}')
        self.cmd(f'cp -rf {self.prefix}/* {self.project.py_external}/Contents/MacOS/{self.project.name}')
        self.chdir(self.project.root)

    def fix_python_dylib_for_ext_resources(self):
        self.chdir(self.prefix)
        self.chmod(self.dylib)
        self.install_name_tool(f'@loader_path/../Resources/{self.project.name}/{self.dylib}', self.dylib)
        self.chdir(self.project.root)

    def cp_python_to_ext_resources(self, arg):
        self.cmd(f'mkdir -p {arg}/Contents/Resources/{self.project.name}')
        self.cmd(f'cp -rf {self.prefix}/* {arg}/Contents/Resources/{self.project.name}')

    def install_python(self):
    # def build(self):
        self.cmd(f'mkdir -p {self.project.lib}')
        self.cmd(f'mkdir -p {self.project.bin}')
        self.cmd(f'cp -rf {self.project.homebrew}/Python {self.prefix}/{self.dylib}')
        self.cmd(f'cp -rf {self.project.homebrew}/lib/${self.project.name}/*.py {self.project.lib}')
        self.cp_pkgs([
            'asyncio',
            'collections',
            'concurrent',
            # 'ctypes',
            # 'curses',
            'dbm',
            'distutils',
            'email',
            'encodings',
            'html',
            'http',
            'importlib',
            'json',
            'lib-dynload',
            'logging',
            'multiprocessing',
            'pydoc_data',
            'sqlite3',
            'unittest',
            'urllib',
            'wsgiref',
            'xml',
            'xmlrpc',
        ])
        self.cmd(f'cp -rf {self.project.homebrew}/include {self.prefix}/include')
        self.cmd(f'rm -rf {self.project.prefix}/lib/{self.dylib}')
        self.cmd(f'rm -rf {self.project.prefix}/lib/pkgconfig')
        self.cmd(f'cp -rf {self.project.homebrew}/Resources/Python.app/Contents/MacOS/Python'
                 f' {self.project.bin}/{self.project.name}')
        self.clean()
        self.ziplib()

    def install_python_pkg(self):
        self.install_python()
        self.fix_python_dylib_for_pkg()

    def install_python_ext(self):
        self.install_python()
        # fix_python_dylib_for_ext
        # fix_python_dylib_for_ext_executable_name
        self.fix_python_dylib_for_ext_resources()
        self.cp_python_to_ext_resources(self.project.py_external)
        # FIXME: for some reason both don't work at the same time!!!
        # you have to pick one.
        # cp_python_to_ext_resources $PYJS_EXTERNAL

    def install_homebrew_pkg(self):
        self.reset_prefix()
        self.install_python_pkg()

    def install_homebrew_ext(self):
        self.install_python_ext()
