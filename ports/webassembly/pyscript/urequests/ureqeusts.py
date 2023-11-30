try:
    import js
except Exception as err:
    raise OSError("This version of urequests can only be used in the browser")

# TODO try to support streaming xhr requests, a-la pyodide-http

from .response import Response
from uio import StringIO

HEADERS_TO_IGNORE = ("user-agent",)


def request(
    method,
    url,
    data=None,
    json=None,
    headers={},
    stream=None,
    auth=None,
    timeout=None,
    parse_headers=True,
):
    from js import XMLHttpRequest

    xhr = XMLHttpRequest.new()
    xhr.withCredentials = False

    if auth is not None:
        import ubinascii

        username, password = auth
        xhr.open(method, url, False, username, password)
    else:
        xhr.open(method, url, False)

    for name, value in headers.items():
        if name.lower() not in HEADERS_TO_IGNORE:
            xhr.setRequestHeader(name, value)

    if timeout:
        xhr.timeout = int(timeout * 1000)

    if json is not None:
        assert data is None
        import ujson

        data = ujson.dumps(json)
        # s.write(b"Content-Type: application/json\r\n")
        xhr.setRequestHeader("Content-Type", "application/json")

    xhr.send(data)

    # Emulates the construction process in the original urequests
    resp = Response(StringIO(xhr.responseText))
    resp.status_code = xhr.status
    resp.reason = xhr.statusText
    resp.headers = xhr.getAllResponseHeaders()

    return resp


# Other methods - head, post, put, patch, delete - are not used by
# mip and therefore not included


def get(url, **kw):
    return request("GET", url, **kw)
