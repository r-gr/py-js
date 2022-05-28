"""

Running this with a particular python version will auto-config py-js.xcconfig


"""

import sysconfig
import os


def xcodebuild(project_path: str, target: str, *preprocessor_flags, **xcconfig_flags):
    """python wrapper around command-line xcodebuild"""
    x_flags = " ".join([f"{k}={repr(v)}" for k,v in xcconfig_flags.items()]) if xcconfig_flags else ''
    p_flags = "GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS {flags}'".format(
        flags=" ".join([f"{k}=1" for k in preprocessor_flags])) if preprocessor_flags else ''
    _cmd = f"xcodebuild -project {repr(project_path)} -target {repr(target)} {x_flags} {p_flags}"
    print(_cmd)
    os.system(_cmd)


flags = dict(
    VERSION = sysconfig.get_config_var('py_version_short'),
    SUFFIX = sysconfig.get_config_var('abiflags'),
)

for t in ['py', 'pyjs']:
    xcodebuild('py-js.xcodeproj', t, **flags)
