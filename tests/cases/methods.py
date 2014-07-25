[
    # Various request methods
    {
        "message": b"GET / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"GET",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"HEAD / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"HEAD",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"POST / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"POST",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"PUT / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"PUT",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"DELETE / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"DELETE",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"TRACE / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"TRACE",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"OPTIONS / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"OPTIONS",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"CONNECT / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"CONNECT",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"PATCH / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"PATCH",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
    {
        "message": b"UNKNOWNMETHOD / HTTP/1.1\r\n\r\n",
        "expected": {
            "request_method": b"UNKNOWNMETHOD",
            "request_uri": b"/",
            "http_version": b"HTTP/1.1",
        },
    },
]
