[
    # A minimal HTTP request
    {
        "message": b"GET / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
]
