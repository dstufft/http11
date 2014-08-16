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

import pytest

import http11.c


def _dict_store_element_callback(data, name):
    def inner(buf, length):
        data[name] = http11.c.ffi.buffer(buf, length)[:]
        return 0
    return inner


def _dict_store_status_code_callback(data, name):
    def inner(status_code):
        data[name] = status_code
        return 0
    return inner


def _dict_store_header_callback(data):
    def inner(name, namelen, value, valuelen):
        name = http11.c.ffi.buffer(name, namelen)[:]
        value = http11.c.ffi.buffer(value, valuelen)[:]

        data.setdefault("headers", {}).setdefault(name, []).append(value)
        return 0
    return inner


@pytest.fixture
def data():
    return {}


@pytest.fixture
def _callbacks():
    """
    This is used to retain a reference to our callback functions.
    """
    return []


@pytest.fixture
def parser(request, data, _callbacks):
    p = http11.c.lib.HTTPParser_create()

    request.addfinalizer(lambda: http11.c.lib.HTTPParser_destroy(p))

    for element in ["request_method", "request_uri", "http_version",
                    "reason_phrase"]:
        _callbacks.append(
            http11.c.ffi.callback(
                "int(const char *buf, size_t length)",
                _dict_store_element_callback(data, element),
            )
        )
        setattr(p, element, _callbacks[-1])

    _callbacks.append(
        http11.c.ffi.callback(
            "int(const unsigned short status_code)",
            _dict_store_status_code_callback(data, "status_code"),
        )
    )
    p.status_code = _callbacks[-1]

    _callbacks.append(
        http11.c.ffi.callback(
            "int(const char *name, size_t namelen, const char *value, "
            "size_t valuelen)",
            _dict_store_header_callback(data),
        )
    )
    p.http_header = _callbacks[-1]

    http11.c.lib.HTTPParser_init(p)

    return p
