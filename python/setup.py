"""Packaging of the cosoco executable as a wheel.

The package carries no extension module, so a single wheel serves every
interpreter; but it is not pure either, since the executable it carries runs
on one platform only. Hence the py3-none-<platform> tag built below, where
the platform is the one of the binary that was copied in -- not the one
building the wheel. The release workflow sets COSOCO_BIN_PLAT accordingly and
runs this once per binary it has produced.
"""

import os
import re
from pathlib import Path

from setuptools import setup
from setuptools.command.bdist_wheel import bdist_wheel
from setuptools.command.install import install

HERE = Path(__file__).resolve().parent


def version():
    """The version of the packaged solver, "2.6.1" for the tag "v2.6.1"."""
    from_workflow = os.environ.get("COSOCO_BIN_VERSION")
    if from_workflow:
        return from_workflow
    # Outside the workflow, follow the version the solver itself reports.
    source = HERE.parent / "main" / "Main.cc"
    found = re.search(r'string\s+version\("([^"]+)"\)', source.read_text())
    if found is None:
        raise RuntimeError("no version found in %s; set COSOCO_BIN_VERSION" % source)
    return found.group(1)


class Install(install):
    def finalize_options(self):
        super().finalize_options()
        # setuptools sees none but Python files and would lay them out as the
        # pure part of a mixed wheel, under cosoco_bin-<version>.data/purelib.
        self.install_lib = self.install_platlib


class BDistWheel(bdist_wheel):
    def finalize_options(self):
        super().finalize_options()
        # An executable is not portable across platforms: this is not a pure
        # wheel, whatever the absence of an extension module suggests.
        self.root_is_pure = False

    def get_tag(self):
        _, _, platform = super().get_tag()
        return "py3", "none", os.environ.get("COSOCO_BIN_PLAT", platform)


setup(version=version(), cmdclass={"install": Install, "bdist_wheel": BDistWheel})
