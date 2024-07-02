import streamlit as st
import time
import numpy as np
# 定义方块的颜色
colors = ['blue', 'green', 'red', 'yellow']



# 创建一个列表存储每个方块的颜色
box_color_false = 'gray'
box_color_true = 'yellow'
width_cube = 300
height_cube = 300
# 创建一个函数,用于更新方块的颜色
# def update_box_color(index):
#     box_colors[index] = colors[index]
#     st.experimental_rerun()
count = None
# with('/home/nuc0428/project/AirTouch/crazyflie_filter_constant/control/sig.csv') open as:
with open('/home/nuc0428/project/AirTouch/crazyflie_filter_constant/control/sig.csv','r') as f:
    count=int(f.read())
# 创建Streamlit应用程序
# 创建Streamlit应用程序
st.set_page_config(layout="wide")  # 设置页面布局为宽屏
# 居中显示标题
st.markdown(
    """
    <div style="display: flex; justify-content: center;">
        <h1>Drone State</h1>
    </div>
    """,
    unsafe_allow_html=True
)

# 显示4个方块
col1, col2, col4, col5 = st.columns(4)
box_color = [box_color_false, box_color_false, box_color_false, box_color_false, box_color_false]

box_text = ['Waiting...', 'Waiting...', 'Waiting...', 'Waiting...', 'Waiting...']



with col5:
    if count>4: #5
        box_color[0] = box_color_true
        box_color[1] = box_color_true
        box_color[2] = box_color_true
        box_color[3] = box_color_true
        box_color[4] = box_color_true
        box_text[4] = 'Landed! Finished!'
        box_text[3] = 'Return to the platform!'
        box_text[2] = 'Exit Edge Detected!'
        box_text[1] = 'Entry Edge Detected!'
        box_text[0] = 'Takeoff!'
    st.markdown(f"<div style='width:{width_cube}px;height:{height_cube}px;background-color:{box_color[4]};display:flex;justify-content:center;align-items:center;'><span style='color:black;font-size:32px;font-weight:bold;'>{box_text[4]}</span></div>", unsafe_allow_html=True)

with col4:
    if count>3: #4
        box_color[0] = box_color_true
        box_color[1] = box_color_true
        box_color[2] = box_color_true
        box_color[3] = box_color_true
        box_text[3] = 'Return to the platform!'
        box_text[2] = 'Exit Edge Detected!'
        box_text[1] = 'Entry Edge Detected!'
        box_text[0] = 'Takeoff!'
    st.markdown(f"<div style='width:{width_cube}px;height:{height_cube}px;background-color:{box_color[3]};display:flex;justify-content:center;align-items:center;'><span style='color:black;font-size:32px;font-weight:bold;'>{box_text[3]}</span></div>", unsafe_allow_html=True)

# with col3:
#     if count>2: #3
#         box_color[0] = box_color_true
#         box_color[1] = box_color_true
#         box_color[2] = box_color_true
#         box_text[2] = 'Exit Edge Detected!'
#         box_text[1] = 'Entry Edge Detected!'
#         box_text[0] = 'Takeoff!'
#     st.markdown(f"<div style='width:{width_cube}px;height:{height_cube}px;background-color:{box_color[2]};display:flex;justify-content:center;align-items:center;'><span style='color:black;font-size:32px;font-weight:bold;'>{box_text[2]}</span></div>", unsafe_allow_html=True)

with col2:
    if count>1: #2
        box_color[0] = box_color_true
        box_color[1] = box_color_true
        box_text[1] = 'Entry Edge\n Detected!'
        box_text[0] = 'Takeoff!'
    st.markdown(f"<div style='width:{width_cube}px;height:{height_cube}px;background-color:{box_color[1]};display:flex;justify-content:center;align-items:center;'><span style='color:black;font-size:32px;font-weight:bold;'>{box_text[1]}</span></div>", unsafe_allow_html=True)

with col1:
    if count>0: #1
        box_color[0] = box_color_true
        box_text[0] = 'Takeoff!'
    st.markdown(f"<div style='width:{width_cube}px;height:{height_cube}px;background-color:{box_color[0]};display:flex;justify-content:center;align-items:center;'><span style='color:black;font-size:32px;font-weight:bold;'>{box_text[0]}</span></div>", unsafe_allow_html=True)



while(1):
    

    
    time.sleep(0.3)
    
    print(count)
    st.rerun()
    

