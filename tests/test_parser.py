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

import pretend

import pytest

from http11.parser import Callback, Error, HTTPParser, MessageType


def test_basic():
    parser = HTTPParser()
    assert parser.parse(b"GET / HTTP/1.1\r\n\r\n") == 18
    assert parser.finished
    assert not parser.errored


@pytest.mark.parametrize(
    ("callback", "expected"),
    [
        (Callback.request_method, b"GET"),
        (Callback.request_uri, b"/foobar/"),
        (Callback.http_version, b"HTTP/1.1"),
    ],
)
def test_element_callback_request(callback, expected):
    func = pretend.call_recorder(lambda data: None)

    parser = HTTPParser()
    parser.callback(callback, func)
    parser.parse(b"GET /foobar/ HTTP/1.1\r\n\r\n")

    assert parser.finished
    assert not parser.errored
    assert func.calls == [pretend.call(expected)]


@pytest.mark.parametrize(
    ("callback", "expected"),
    [
        (Callback.reason_phrase, b"OK"),
        (Callback.http_version, b"HTTP/1.1"),
    ],
)
def test_element_callback_response(callback, expected):
    func = pretend.call_recorder(lambda data: None)

    parser = HTTPParser()
    parser.callback(callback, func)
    parser.parse(b"HTTP/1.1 200 OK\r\n\r\n")

    assert parser.finished
    assert not parser.errored
    assert func.calls == [pretend.call(expected)]


def test_status_code_callback():
    func = pretend.call_recorder(lambda status_code: None)

    parser = HTTPParser()
    parser.callback(Callback.status_code, func)
    parser.parse(b"HTTP/1.1 200 OK\r\n\r\n")

    assert parser.finished
    assert not parser.errored
    assert func.calls == [pretend.call(200)]


def test_header_callback():
    func = pretend.call_recorder(lambda name, value: None)

    parser = HTTPParser()
    parser.callback(Callback.http_header, func)
    parser.parse(b"HTTP/1.1 200 OK\r\nFoo: Bar\r\n\r\n")

    assert parser.finished
    assert not parser.errored
    assert func.calls == [pretend.call(b"Foo", b"Bar")]


@pytest.mark.parametrize(
    ("callback", "data"),
    [
        (Callback.request_method, b"GET / HTTP/1.1\r\n\r\n"),
        (Callback.request_uri, b"GET / HTTP/1.1\r\n\r\n"),
        (Callback.http_version, b"GET / HTTP/1.1\r\n\r\n"),
        (Callback.reason_phrase, b"HTTP/1.1 200 OK\r\n\r\n"),
        (Callback.status_code, b"HTTP/1.1 200 OK\r\n\r\n"),
        (Callback.http_header, b"HTTP/1.1 200 OK\r\nFoo: Bar\r\n\r\n"),
    ],
)
def test_callback_error(callback, data):
    @pretend.call_recorder
    def raiser(*args, **kwargs):
        raise Exception

    parser = HTTPParser()
    parser.callback(callback, raiser)
    parser.parse(data)

    assert parser.finished
    assert parser.errored
    assert parser.error == Error.General


def test_error_is_none():
    parser = HTTPParser()

    assert not parser.finished
    assert not parser.errored
    assert parser.error is None

    parser.parse(b"HTTP/2.0 200 OK\r\n\r\n")

    assert parser.finished
    assert parser.errored
    assert parser.error == Error.BadVersion


def test_unknown_callback():
    parser = HTTPParser()

    with pytest.raises(ValueError):
        parser.callback("wat", lambda: None)


def test_callback_decorator():
    parser = HTTPParser()

    @parser.callback(Callback.http_version)
    @pretend.call_recorder
    def noop(*args, **kwargs):
        pass

    parser.parse(b"HTTP/1.1 200 OK\r\n\r\n")

    assert parser.finished
    assert not parser.errored


@pytest.mark.parametrize(
    ("message", "mtype"),
    [
        (b"GET / HTTP/1.1\r\n\r\n", MessageType.Request),
        (b"HTTP/1.1 200 OK\r\n\r\n", MessageType.Response),
    ],
)
def test_callback_message_type(message, mtype):
    parser = HTTPParser()
    assert parser.type is None
    parser.parse(message)
    assert parser.type == mtype


def test_message_reset():
    parser = HTTPParser()

    assert not parser.finished
    assert not parser.errored
    assert parser.error is None
    assert parser.type is None

    parser.parse(b"HTTP/1.1 200 OK\r\n\r\n")

    assert parser.finished
    assert not parser.errored
    assert parser.error is None
    assert parser.type is MessageType.Response

    parser.reset()

    assert not parser.finished
    assert not parser.errored
    assert parser.error is None
    assert parser.type is None

    parser.parse(b"GET / HTTP/1.1\r\n\r\n")

    assert parser.finished
    assert not parser.errored
    assert parser.error is None
    assert parser.type is MessageType.Request
