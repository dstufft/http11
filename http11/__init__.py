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

import binascii
import os.path
import sys

from cffi import FFI
from cffi.verifier import Verifier


# Locate our source directory for our C files.
SRC_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "c"))


# Write out our cdef to bind data to our FFI instance
CDEF = """
    #define EINVALIDMSG ...
    #define EHTTP505 ...

    typedef struct _HTTPParserState *HTTPParserState;

    typedef int (*element_cb)(const char *buf, size_t length);

    typedef int (*status_code_cb)(const unsigned short status_code);

    typedef int (*header_cb)(const char *name,
                             size_t namelen,
                             const char *value,
                             size_t valuelen);

    typedef struct HTTPParser {
        /* Public State */
        int finished;
        int error;

        /* Callback Methods */
        element_cb request_method;
        element_cb request_uri;
        element_cb http_version;
        element_cb reason_phrase;

        status_code_cb status_code;

        header_cb http_header;

        /* Internal State */
        HTTPParserState state;

    } HTTPParser;

    HTTPParser *HTTPParser_create();
    void HTTPParser_init(HTTPParser *parser);
    size_t HTTPParser_execute(HTTPParser *parser,
                              const char *buf,
                              size_t offset,
                              size_t length);
    void HTTPParser_destroy(HTTPParser *parser);
"""

SOURCE = """
    #include <http11.h>
"""

# Build our FFI instance
ffi = FFI()
ffi.cdef(CDEF)


def create_modulename(cdef_source, source, sys_version):
    """
    cffi creates a modulename internally that incorporates the cffi version.
    This will cause Fenrir's wheels to break when the version of cffi
    the user has does not match what was used when building the wheel. To
    resolve this we build our own modulename that uses most of the same code
    from cffi but elides the version key.
    """
    key = "\x00".join([sys_version[:3], source, cdef_source])
    key = key.encode("utf-8")
    k1 = hex(binascii.crc32(key[0::2]) & 0xffffffff)
    k1 = k1.lstrip("0x").rstrip("L")
    k2 = hex(binascii.crc32(key[1::2]) & 0xffffffff)
    k2 = k2.lstrip("0").rstrip("L")
    return "_cffi_{0}{1}".format(k1, k2)


# Construct a Verifier manually, this will prevent the ffi instance from
# attempting to load the library, which would trigger a compile normally if it
# can't be loaded, which we want to delay so it doesn't happen on import. This
# will enable us to import this module, and use it in our setup.py to get the
# Extension object to allow distutils to build it normally.
ffi.verifier = Verifier(
    ffi,

    SOURCE,

    # This needs to match the value in setup.py
    ext_package="http11",

    # Fix the fact that CFFI doesn't sanely work when you don't have the exact
    # version installed that a library was built against.
    modulename=create_modulename(CDEF, SOURCE, sys.version),

    # We want to compile the http_parser.c instead of trying to link against it
    # or anything like that.
    sources=[
        os.path.join(SRC_DIR, "http11.c"),
    ],

    # We need to include the bundled dir so that we can include the header
    # files located in it.
    include_dirs=[
        SRC_DIR,
    ],
)


class Library:

    def __init__(self, ffi):
        self.ffi = ffi
        self._lib = None

        # This prevents the compile_module() from being called, the module
        # should have been compiled by setup.py
        def _compile_module(*args, **kwargs):
            raise RuntimeError("Cannot compile module during runtime")
        self.ffi.verifier.compile_module = _compile_module
        self.ffi.verifier._compile_module = _compile_module

    @property
    def lib(self):
        if self._lib is None:
            self._lib = self.ffi.verifier.load_library()
        return self._lib

    def __getattr__(self, name):
        return getattr(self.lib, name)


lib = Library(ffi)
