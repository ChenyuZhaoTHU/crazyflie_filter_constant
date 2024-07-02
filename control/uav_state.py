import asyncio
import json
import websockets

# 假设这些是无人机的当前状态
drone_status = {
    "x": 100,
    "y": 200,
    "altitude": 50,
    "battery": 80
}

async def send_drone_status(websocket, path):
    async for message in websocket:
        # 更新无人机状态
        drone_status["x"] += 1
        drone_status["y"] += 1
        drone_status["altitude"] -= 1
        drone_status["battery"] -= 1

        # 发送状态信息
        await websocket.send(json.dumps(drone_status))
        await asyncio.sleep(1)

start_server = websockets.serve(send_drone_status, "localhost", 8765)
print("WebSocket 服务器已启动,正在监听 localhost:8765")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
