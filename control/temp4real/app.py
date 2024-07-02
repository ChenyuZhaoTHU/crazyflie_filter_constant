import random


import streamlit as st
import time

# Create an empty container
container = st.empty()


# 使用列表来存储所有的数据
data_str = "实时转录："
# Update the container every second
while True:
    # Get new data here
    new_data =  random.random()
    
    # 将新数据添加到列表中
    data_str = data_str + str(new_data)

    # 添加所有数据到容器中
    # container.write(f"""# {data_str}""")
    # container.write(data_str)
    
    # Write the new data to the container
    container.write(new_data)
    
    # Wait for 1 second before checking for new data again
    time.sleep(1)