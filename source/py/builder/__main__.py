#!/usr/bin/env python3
"""builder -- build python3 from source (currently for macos)

    usage: pybuild.py [-h] [-v] {all,framework,shared,static} ...

    pybuild: builds python from src

    optional arguments:
      -h, --help            show this help message and exit
      -v, --version         show program's version number and exit

    subcommands:
      valid subcommands
                            additional help
        all                 build all variations
        framework           build framework python
        shared              build shared python
        static              build static python

"""
from .builders import (FrameworkPythonBuilder, HomebrewBuilder,
                       SharedPythonBuilder, StaticPythonBuilder)
from .utils.cli import Commander, option, option_group

# ------------------------------------------------------------------------------
# Commandline interface

common_options = option_group(
    option("-d",
           "--download",
           action="store_true",
           help="download python build/downloads"),
    option("-r", "--reset", action="store_true", help="reset python build"),
    option("-i",
           "--install",
           action="store_true",
           help="install python to build/lib"),
    option("-b",
           "--build",
           action="store_true",
           help="build python in build/src"),
    option("-c",
           "--clean",
           action="store_true",
           help="clean python in build/src"),
    option("-z", "--ziplib", action="store_true", help="zip python library"),
)


class Application(Commander):
    """builder: builds the py-js max external and python from source."""
    name = 'pybuild'
    epilog = ''
    version = '0.1'
    default_args = ['--help']

    def dispatch1(self, builder_class, args):
        """generic dispatcher"""
        builder = builder_class()
        if args.download:
            builder.download()
        elif args.reset:
            builder.reset()
        elif args.install:
            builder.install()
        elif args.build:
            builder.build()
        elif args.clean:
            builder.clean()
        elif args.ziplib:
            builder.ziplib()

    def dispatch(self, builder_class, args):
        """generic argument dispatcher"""
        builder = builder_class()
        for key in vars(args):
            if key == 'func':
                continue
            if getattr(args, key):
                try:
                    getattr(builder, key)()
                except AttributeError:
                    print(builder, 'has no method', key)

    @common_options
    def do_py_static(self, args):
        """build static python"""
        self.dispatch(StaticPythonBuilder, args)

    @common_options
    def do_py_shared(self, args):
        """build shared python"""
        self.dispatch(SharedPythonBuilder, args)

    @common_options
    def do_py_framework(self, args):
        """build framework python"""
        self.dispatch(FrameworkPythonBuilder, args)

    @common_options
    def do_py_all(self, args):
        """build all python variations"""
        for builder_class in [FrameworkPythonBuilder, SharedPythonBuilder, StaticPythonBuilder]:
            self.dispatch(builder_class, args)

    @common_options
    def do_pyjs_sys(self, args):
        """build non-portable pyjs package from homebrew python"""
        b = HomebrewBuilder()
        b.install_homebrew_sys()

    @common_options
    def do_pyjs_pkg(self, args):
        """build portable pyjs package from homebrew python"""
        b = HomebrewBuilder()
        b.install_homebrew_pkg()

    @common_options
    def do_pyjs_ext(self, args):
        """build portable pyjs externals from homebrew python"""
        b = HomebrewBuilder()
        # b.install_homebrew_ext()
        b.install_homebrew_ext_py()
        b.install_homebrew_ext_pyjs()

    @common_options
    def do_test(self, args):
        """interactive testing shell"""
        from IPython import embed
        embed(colors="neutral")




if __name__ == '__main__':
    app = Application()
    app.cmdline()
