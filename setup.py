#!/usr/bin/env python
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from distutils.command.build import build

from setuptools import find_packages, setup
from setuptools.command.install import install


CFFI_DEPENDENCY = "cffi>=0.8"


def get_ext_modules():
    import http11.c
    return [http11.c.ffi.verifier.get_extension()]


class CFFIBuild(build):
    """
    This class exists, instead of just providing ``ext_modules=[...]`` directly
    in ``setup()`` because importing cryptography requires we have several
    packages installed first.

    By doing the imports here we ensure that packages listed in
    ``setup_requires`` are already installed.
    """

    def finalize_options(self):
        self.distribution.ext_modules = get_ext_modules()
        build.finalize_options(self)


class CFFIInstall(install):
    """
    As a consequence of CFFIBuild and it's late addition of ext_modules, we
    need the equivalent for the ``install`` command to install into platlib
    install-dir rather than purelib.
    """

    def finalize_options(self):
        self.distribution.ext_modules = get_ext_modules()
        install.finalize_options(self)


meta = {}
with open("http11/__about__.py") as fp:
    exec(fp.read(), meta)


with open("README.rst") as fp:
    long_description = fp.read()


setup(
    name=meta["__title__"],
    version=meta["__version__"],

    description=meta["__summary__"],
    long_description=long_description,
    license=meta["__license__"],
    url=meta["__url__"],

    author=meta["__author__"],
    author_email=meta["__email__"],

    classifiers=[
        "License :: OSI Approved :: Apache Software License",

        "Programming Language :: Python",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 2.6",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.2",
        "Programming Language :: Python :: 3.3",
        "Programming Language :: Python :: 3.4",
        "Programming Language :: Python :: Implementation :: CPython",
        "Programming Language :: Python :: Implementation :: PyPy",
    ],

    packages=find_packages(),

    install_requires=[
        CFFI_DEPENDENCY,
        "enum34",
    ],

    setup_requires=[
        CFFI_DEPENDENCY,
    ],

    # These are needed so that CFFI can correctly function
    zip_safe=False,
    ext_package="http11",
    cmdclass={
        "build": CFFIBuild,
        "install": CFFIInstall,
    }
)
