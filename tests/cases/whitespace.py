[
    {
        "message": b"GET  \t\t    /    \t   HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"HTTP/1.1 \t\r\t\t      200\t\t\t \t    OK\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 200,
            "reason_phrase": b"OK",
        }
    },
    {
        "message": b"HTTP/1.1       \t\t200         \t\t\t\t\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 200,
        }
    },
]
