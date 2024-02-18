#!/usr/bin/env python3

import asyncio
import websockets

# Version 1
async def echo(websocket):
    async for message in websocket:
        print(f"< {message}")
        await websocket.send(message)
        print(f"> {message}")

async def main():
    async with websockets.serve(echo, "localhost", 5000):
        await asyncio.Future()  # run forever

asyncio.run(main())
