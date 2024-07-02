import socket

# 创建套接字
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 连接到服务器
server_address = ('localhost', 12345)
client_socket.connect(server_address)

# 循环发送数据给服务器
data_to_send = "Hello, Server!"
while data_to_send:
    client_socket.send(data_to_send.encode())
    data_to_send = input("Enter data to send (or press Enter to quit): ")

# 关闭套接字
client_socket.close()