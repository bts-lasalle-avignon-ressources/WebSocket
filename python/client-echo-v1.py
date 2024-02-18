#!/usr/bin/env python3

import asyncio
from websockets.sync.client import connect

def hello():
    with connect("ws://localhost:5000") as websocket:
        message = f"Hello world!"
        print(f"> {message}")
        websocket.send(message)
        message = websocket.recv()
        print(f"< {message}")

hello()
