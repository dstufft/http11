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

import os.path

from cffi import FFI
from cffi.verifier import Verifier


# Locate our source directory for our C files.
SRC_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "c"))


# Write out our cdef to bind data to our FFI instance
CDEF = """
    typedef struct _HTTPParserState *HTTPParserState;

    typedef void (*element_cb)(const char *buf, size_t len);

    typedef struct HTTPParser {
        /* Public State */
        int finished;
        int error;

        /* Callback Methods */
        element_cb request_method;

        /* Internal State */
        HTTPParserState state;

    } HTTPParser;

    HTTPParser *HTTPParser_create();
    void HTTPParser_init(HTTPParser *parser);
    size_t HTTPParser_execute(HTTPParser *parser, const char *data,
                               size_t len, size_t off);
"""

SOURCE = """
    #include <http11.h>
"""

# Build our FFI instance
ffi = FFI()
ffi.cdef(CDEF)


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

lib = ffi.verifier.load_library()
