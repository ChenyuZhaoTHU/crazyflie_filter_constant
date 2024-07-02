

import tkinter as tk
import asyncio
import json
import websockets

# 创建主窗口
root = tk.Tk()
root.title("无人机状态监控")

# 创建标签用于显示状态
status_label = tk.Label(root, text="正在连接...", font=("Arial", 16))
status_label.pack(pady=20)

async def update_status():
    async with websockets.connect("ws://localhost:8765") as websocket:
        print('sucess connect')
        while True:
            status_data = json.loads(await websocket.recv())
            status_text = f"位置: ({status_data['x']}, {status_data['y']}, {status_data['altitude']})\n"
            status_text += f"电池电量: {status_data['battery']}%"
            status_label.config(text=status_text)
            await asyncio.sleep(0.1)

async def main():
    await asyncio.gather(
        update_status(),
        run_tkinter()
    )

async def run_tkinter():
    root.mainloop()

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())