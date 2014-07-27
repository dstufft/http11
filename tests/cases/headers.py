[
    {
        "message": b"GET / HTTP/1.1\r\nFoo: Bar\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar"],
            },
        },
    },
    {
        "message": b"GET / HTTP/1.1\r\nFoo: Bar\r\nFoo: Wat\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar", b"Wat"],
            },
        },
    },
    {
        "message": b"GET / HTTP/1.1\r\nFoo: Bar     \r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar"],
            },
        },
    },
    {
        "message": b"GET / HTTP/1.1\r\nFoo:Bar\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar"],
            },
        },
    },
    {
        "message": b"GET / HTTP/1.1\r\nFoo:    Bar     \r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar"],
            },
        },
    },
    {
        "message": b"GET / HTTP/1.1\r\nFoo: \r\n    Bar\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar"],
            },
        },
    },
    {
        "message": b"GET / HTTP/1.1\r\nFoo: \r\n    Bar\r\n\t\t  \tF\r\n\r\n",
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
        "message": b"GET / HTTP/1.1\r\nFoo: \r\n    Bar\r\n\t\t  \tF\r\n"
                   b"Another: One\r\nAnother: Two\r\n Three?\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
            "headers": {
                b"Foo": [b"Bar F"],
                b"Another": [b"One", b"Two Three?"],
            },
        },
    },
]
