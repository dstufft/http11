[
    {
        "message": b"HTTP/1.1 100 Continue\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 100,
            "reason_phrase": b"Continue",
        }
    },
    {
        "message": b"HTTP/1.1 101 Switching Protocols\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 101,
            "reason_phrase": b"Switching Protocols",
        }
    },
    {
        "message": b"HTTP/1.1 200 OK\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 200,
            "reason_phrase": b"OK",
        }
    },
    {
        "message": b"HTTP/1.1 201 Created\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 201,
            "reason_phrase": b"Created",
        }
    },
    {
        "message": b"HTTP/1.1 202 Accepted\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 202,
            "reason_phrase": b"Accepted",
        }
    },
    {
        "message": b"HTTP/1.1 203 Non-Authoritative Information\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 203,
            "reason_phrase": b"Non-Authoritative Information",
        }
    },
    {
        "message": b"HTTP/1.1 204 No Content\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 204,
            "reason_phrase": b"No Content",
        }
    },
    {
        "message": b"HTTP/1.1 205 Reset Content\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 205,
            "reason_phrase": b"Reset Content",
        }
    },
    {
        "message": b"HTTP/1.1 206 Partial Content\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 206,
            "reason_phrase": b"Partial Content",
        }
    },
    {
        "message": b"HTTP/1.1 300 Multiple Choices\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 300,
            "reason_phrase": b"Multiple Choices",
        }
    },
    {
        "message": b"HTTP/1.1 301 Moved Permanently\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 301,
            "reason_phrase": b"Moved Permanently",
        }
    },
    {
        "message": b"HTTP/1.1 302 Found\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 302,
            "reason_phrase": b"Found",
        }
    },
    {
        "message": b"HTTP/1.1 303 See Other\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 303,
            "reason_phrase": b"See Other",
        }
    },
    {
        "message": b"HTTP/1.1 304 Not Modified\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 304,
            "reason_phrase": b"Not Modified",
        }
    },
    {
        "message": b"HTTP/1.1 305 Use Proxy\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 305,
            "reason_phrase": b"Use Proxy",
        }
    },
    {
        "message": b"HTTP/1.1 306 (Unused)\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 306,
            "reason_phrase": b"(Unused)",
        }
    },
    {
        "message": b"HTTP/1.1 307 Temporary Redirect\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 307,
            "reason_phrase": b"Temporary Redirect",
        }
    },
    {
        "message": b"HTTP/1.1 400 Bad Request\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 400,
            "reason_phrase": b"Bad Request",
        }
    },
    {
        "message": b"HTTP/1.1 401 Unauthorized\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 401,
            "reason_phrase": b"Unauthorized",
        }
    },
    {
        "message": b"HTTP/1.1 402 Payment Required\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 402,
            "reason_phrase": b"Payment Required",
        }
    },
    {
        "message": b"HTTP/1.1 403 Forbidden\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 403,
            "reason_phrase": b"Forbidden",
        }
    },
    {
        "message": b"HTTP/1.1 404 Not Found\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 404,
            "reason_phrase": b"Not Found",
        }
    },
    {
        "message": b"HTTP/1.1 405 Method Not Allowed\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 405,
            "reason_phrase": b"Method Not Allowed",
        }
    },
    {
        "message": b"HTTP/1.1 406 Not Acceptable\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 406,
            "reason_phrase": b"Not Acceptable",
        }
    },
    {
        "message": b"HTTP/1.1 407 Proxy Authentication Required\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 407,
            "reason_phrase": b"Proxy Authentication Required",
        }
    },
    {
        "message": b"HTTP/1.1 408 Request Timeout\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 408,
            "reason_phrase": b"Request Timeout",
        }
    },
    {
        "message": b"HTTP/1.1 409 Conflict\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 409,
            "reason_phrase": b"Conflict",
        }
    },
    {
        "message": b"HTTP/1.1 410 Gone\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 410,
            "reason_phrase": b"Gone",
        }
    },
    {
        "message": b"HTTP/1.1 411 Length Required\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 411,
            "reason_phrase": b"Length Required",
        }
    },
    {
        "message": b"HTTP/1.1 412 Precondition Failed\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 412,
            "reason_phrase": b"Precondition Failed",
        }
    },
    {
        "message": b"HTTP/1.1 413 Request Entity Too Large\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 413,
            "reason_phrase": b"Request Entity Too Large",
        }
    },
    {
        "message": b"HTTP/1.1 414 Request-URI Too Long\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 414,
            "reason_phrase": b"Request-URI Too Long",
        }
    },
    {
        "message": b"HTTP/1.1 415 Unsupported Media Type\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 415,
            "reason_phrase": b"Unsupported Media Type",
        }
    },
    {
        "message": b"HTTP/1.1 416 Requested Range Not Satisfiable\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 416,
            "reason_phrase": b"Requested Range Not Satisfiable",
        }
    },
    {
        "message": b"HTTP/1.1 417 Expectation Failed\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 417,
            "reason_phrase": b"Expectation Failed",
        }
    },
    {
        "message": b"HTTP/1.1 500 Internal Server Error\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 500,
            "reason_phrase": b"Internal Server Error",
        }
    },
    {
        "message": b"HTTP/1.1 501 Not Implemented\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 501,
            "reason_phrase": b"Not Implemented",
        }
    },
    {
        "message": b"HTTP/1.1 502 Bad Gateway\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 502,
            "reason_phrase": b"Bad Gateway",
        }
    },
    {
        "message": b"HTTP/1.1 503 Service Unavailable\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 503,
            "reason_phrase": b"Service Unavailable",
        }
    },
    {
        "message": b"HTTP/1.1 504 Gateway Timeout\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 504,
            "reason_phrase": b"Gateway Timeout",
        }
    },
    {
        "message": b"HTTP/1.1 505 HTTP Version Not Supported\r\n\r\n",
        "expected": {
            "http_version": b"HTTP/1.1",
            "status_code": 505,
            "reason_phrase": b"HTTP Version Not Supported",
        }
    },
]
