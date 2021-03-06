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

    # A minimal HTTP response
    {
        "message": b"HTTP/1.1 200 OK\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 200,
            "reason_phrase": b"OK",
        }
    },

    # A minimal HTTP response with the optional reason phrase omitted
    {
        "message": b"HTTP/1.1 200\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 200,
        }
    },
]
