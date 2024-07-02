import socket

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

# 关闭套接字
client_socket.close()
server_socket.close()