from datetime import datetime
import asyncio
import os
import sys
import socket
import time
from urllib.parse import unquote as urldecode
from typing import Optional

LISTEN_PORT = int(os.getenv('LISTENER_PORT', '8080'))
LISTEN_IP = os.getenv('LISTENER_IP', '0.0.0.0')

class RTBListenerProtocol(asyncio.Protocol):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.transport = None
        self.peername = None
        self.request_count = 0

        self.connection_time = None

    def connection_made(self, transport):
        self.transport = transport

    def data_received(self, data):
        request_start = time.time()

        if data[:4] != b'GET ':
            print("Invalid request")
            self.transport.close()
            return

        self.request_count += 1

        request, *_headers = data.splitlines()

        _method, path, _protocol = request.split(b' ')

        if path == b"/ready":
            self.write_response(b"1")

            return

        if path[:14] != b'/category?url=':
            print("Unknown request: %s" % path)
            self.write_response(b'')

            return

        url = urldecode(path[14:].decode("ascii"))
        result = b''

        self.write_response(result)

    def write_response(self, data):

        self.transport.write(b"HTTP/1.1 200 OK\r\n"
                             b"Connection: keep-alive\r\n"
                             b"Content-Type: text/plain; charset=ascii\r\n"
                             b"Content-Length: " + str(len(data) + 2).encode('ascii') + b"\r\n"
                             b"\r\n" + data + b"\r\n")


async def async_server() -> None:
    loop = asyncio.get_running_loop()

    server = await loop.create_server(RTBListenerProtocol, LISTEN_IP, LISTEN_PORT)

    address = server.sockets[0].getsockname()
    print("Listening on: %s" % (address, ))

    try:
        # Run both the actual listener and the watchdog loop
        tasks = asyncio.gather(server.serve_forever(),
                               loop=loop)

        await tasks
    except Exception:
        tasks.cancel()

        raise

def async_main():

    try:
        asyncio.run(async_server())
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    async_main()
