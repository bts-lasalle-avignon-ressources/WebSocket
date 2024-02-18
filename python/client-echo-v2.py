#!/usr/bin/env python3
import asyncio
import websockets

async def hello():
    async with websockets.connect("ws://localhost:5000") as websocket:
        message = f"Hello world!"
        print(f"> {message}")
        await websocket.send(message)
        message = await websocket.recv()
        print(f"< {message}")

asyncio.get_event_loop().run_until_complete(hello())