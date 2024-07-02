import matplotlib.pyplot as plt
import threading
import time
import socket
# 存储传感器数据的列表
sensor_data = []

# 线程函数，用于读取传感器数据并更新图表
def read_sensor_thread():
        # 创建套接字
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # 绑定IP和端口
    server_address = ('localhost', 12345)
    server_socket.bind(server_address)

    # 监听连接
    server_socket.listen(1)

    # 等待客户端连接
    print('等待客户端连接...')
    client_socket, client_address = server_socket.accept()
    print('客户端已连接:', client_address)

    # 循环接收来自客户端的数据
    buffer_size = 1024
    while True:
        received_data = client_socket.recv(buffer_size).decode()
        if not received_data:
            break
        print(f"The data from the client is: {received_data}")

   
        # 读取传感器数据
        # data = read_sensor_data()
        
        # 将数据添加到列表中
        sensor_data.append(received_data)
        
        # 等待一段时间
        time.sleep(0.04)

# 函数模拟传感器数据的读取
def read_sensor_data():
    # 在这里实现读取传感器数据的逻辑
    # 这里仅返回一个随机数，您需要根据实际情况修改此处的代码
    import random
    return random.random()

# 创建一个线程用于读取传感器数据
thread = threading.Thread(target=read_sensor_thread)
thread.daemon = True
thread.start()

# 创建一个图表窗口
plt.figure()

# 主循环，用于实时绘制图表
while True:
    # 清除图表
    plt.clf()
    
    # 绘制传感器数据
    plt.plot(sensor_data)
    
    # 刷新图表
    plt.pause(0.04)