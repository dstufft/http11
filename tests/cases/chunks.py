[
    {
        "message": [
            b"GET / "
            b"HTTP/1.1\r\n\r\n",
        ],
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": [
            b"GET / HTTP/1.1\r\nFoo: \r\n"
            b"    Bar\r\n\t\t  \tF\r\n\r\n",
        ],
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar F"],
            },
        },
    },
    {
        "message": [
            b"HTTP/1.1 2"
            b"00 OK\r\n\r\n",
        ],
        "expected": {
            "status_code": 200,
            "reason_phrase": b"OK",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": [
            b"GET / HTTP/1.1\r\n"
            b"Foo: \r\n    Bar\r\n\t\t  ",
            b"\tF\r\n\r\n",
        ],
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar F"],
            },
        },
    },
    {
        "message": [
            b"GET / HTTP/1.1\r\n"
            b"Foo: \r\n    Bar\r\n\t\t  \tF\r\n\r\n",
        ],
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar F"],
            },
        },
    },
]
