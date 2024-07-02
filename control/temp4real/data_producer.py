import random
import time

def generate_data():
    data = [random.randint(0, 100) for _ in range(4)]
    return data

while True:
    data = generate_data()
    print(data)
    time.sleep(2)