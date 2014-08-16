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

import enum

from http11 import c


def _element_callback(func):
    def inner(buf, length):
        try:
            func(c.ffi.buffer(buf, length)[:])
        except:
            # TODO: Do we really want this to be a bare except?
            return c.lib.EERROR
        else:
            return 0
    return inner


def _status_code_callback(func):
    def inner(status_code):
        try:
            func(status_code)
        except:
            # TODO: Do we really want this to be a bare except?
            return c.lib.EERROR
        else:
            return 0
    return inner


def _header_callback(func):
    def inner(name, namelen, value, valuelen):
        try:
            func(
                c.ffi.buffer(name, namelen)[:],
                c.ffi.buffer(value, valuelen)[:],
            )
        except:
            # TODO: Do we really want this to be a bare except?
            return c.lib.EERROR
        else:
            return 0
    return inner


class Callback(enum.Enum):

    request_method = "request_method"
    request_uri = "request_uri"
    http_version = "http_version"
    reason_phrase = "reason_phrase"
    status_code = "status_code"
    http_header = "http_header"


class Error(enum.IntEnum):

    General = c.lib.EERROR
    EOF = c.lib.EEOF
    InvalidMessage = c.lib.EINVALIDMSG
    BadVersion = c.lib.EBADVERSION


class MessageType(enum.IntEnum):

    Request = c.lib.REQUEST
    Response = c.lib.RESPONSE


class HTTPParser(object):

    def __init__(self):
        self.callbacks = {}
        self.parser = c.ffi.gc(
            c.lib.HTTPParser_create(),
            c.lib.HTTPParser_destroy,
        )
        self.reset()

    @property
    def finished(self):
        return bool(self.parser.finished)

    @property
    def errored(self):
        return bool(self.parser.error)

    @property
    def error(self):
        if not self.errored:
            return

        return Error(self.parser.error)

    @property
    def type(self):
        try:
            return MessageType(self.parser.type)
        except ValueError:
            return

    def reset(self):
        c.lib.HTTPParser_init(self.parser)

    def callback(self, cname, func=None):
        def _register(func):
            if cname in set([
                    Callback.request_method,
                    Callback.request_uri,
                    Callback.http_version,
                    Callback.reason_phrase]):
                self.callbacks[cname] = c.ffi.callback(
                    "int(const char *buf, size_t length)",
                    _element_callback(func),
                )
            elif cname is Callback.status_code:
                self.callbacks[cname] = c.ffi.callback(
                    "int(const unsigned short status_code)",
                    _status_code_callback(func),
                )
            elif cname is Callback.http_header:
                self.callbacks[cname] = c.ffi.callback(
                    "int(const char *name, size_t namelen, "
                    "const char *value, size_t valuelen)",
                    _header_callback(func),
                )
            else:
                raise ValueError("Unknown callback name.")

            setattr(self.parser, cname.value, self.callbacks[cname])

            return func

        if func is not None:
            _register(func)
        else:
            return _register

    def parse(self, data):
        length = len(data) if data is not None else 0
        return c.lib.HTTPParser_execute(self.parser, data, 0, length)
