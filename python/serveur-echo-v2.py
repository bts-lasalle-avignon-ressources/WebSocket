#!/usr/bin/env python3

import asyncio
import websockets

# Version 2
async def echo(websocket):
    donneesRecues = await websocket.recv()
    print(f"< {donneesRecues}")
    await websocket.send(donneesRecues)
    print(f"> {donneesRecues}")

serveur = websockets.serve(echo, "localhost", 5000)
asyncio.get_event_loop().run_until_complete(serveur)
asyncio.get_event_loop().run_forever()
