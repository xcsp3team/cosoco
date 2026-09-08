"""The cosoco constraint solver, shipped as a prebuilt executable.

cosoco is written in C++, and this package holds no binding: only the binary,
to be run as a subprocess on an XCSP3 instance.

    import subprocess
    from cosoco_bin import PATH
    subprocess.run([PATH, "instance.xml", "-model=2"])

Note that cosoco expects the instance first, before any option. The executable
is installed as the command `cosoco` as well.
"""

import os
import sys
from importlib.metadata import PackageNotFoundError, version as _version

__all__ = ["PATH", "main"]

#: Absolute path of the cosoco executable.
PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "cosoco")

try:
    __version__ = _version("cosoco-bin")
except PackageNotFoundError:  # imported from a source tree, not installed
    __version__ = "unknown"


def main():
    """Entry point of the `cosoco` command: hand the process over to the solver."""
    os.execv(PATH, [PATH] + sys.argv[1:])
