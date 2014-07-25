[
    {
        "message": b"GET / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"GET /foo/bar/ HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/foo/bar/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"GET /foo%20bar/ HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/foo%20bar/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"GET /foo/bar/?q=wat HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/foo/bar/?q=wat",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"GET /foo/bar/?q=wat+wat HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/foo/bar/?q=wat+wat",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"GET /#fragment HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/#fragment",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"GET /invalid spaces but should parse/ HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/invalid spaces but should parse/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"OPTIONS * HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"OPTIONS",
            "request_uri": b"*",
            "http_version": b"HTTP/1.1",
        },
    },
]
