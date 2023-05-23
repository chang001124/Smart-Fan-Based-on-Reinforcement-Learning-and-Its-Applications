# -*- coding: utf-8 -*-
"""
Created on Thu May  4 22:27:53 2023

@author: angel
"""
import pandas as pd
import matplotlib.pyplot as plt
from scipy.stats import linregress
# 讀取 Excel 檔案
df = pd.read_excel('stupid_fan.xlsx')

# 假設溫度資料儲存在 '溫度' 欄位中
temperatures = df['temp']
fan_speed = df['fan_speed']
time = df['id']
#plt.plot(time,temperatures )
plt.plot(time,fan_speed )
slope, intercept, r_value, p_value, std_err = linregress(time, fan_speed)
#plt.scatter(temperatures,fan_speed)
plt.plot(time, slope*time + intercept, color='red')
# 設定圖表標題和軸標籤
#plt.title("fan_speed vs temperatures")
plt.title("PWM vs times")
#plt.xlabel("temperatures")
plt.xlabel("times")
plt.ylabel("PWM")
plt.show()

plt.axhline(y=37, c="w", ls="--", lw=2)
plt.axhline(y=30, c="w", ls="--", lw=2)
#plt.axhline(y=30.97, c="r", ls="--", lw=2)
plt.axhline(y=34.36, c="r", ls="--", lw=2)
plt.plot(time,temperatures )
plt.title("fan with Q-learning")
plt.xlabel("times")
plt.ylabel("temperatures")
plt.show()

"""plt.plot(time,fan_speed )
plt.title("fan_speed vs times")
plt.xlabel("times")
plt.ylabel("fan_speed")
# 顯示圖表
plt.show()"""

"""plt.scatter( temperatures,fan_speed)
slope, intercept, r_value, p_value, std_err = linregress(temperatures, fan_speed)
#plt.scatter(temperatures,fan_speed)
print("斜率：", slope)
print("截距：", intercept)
print("相關係數：", r_value)
print("p值：", p_value)
print("標準誤：", std_err)
plt.plot(temperatures, slope*temperatures + intercept, color='red')
plt.title("PWM vs temperature")
plt.xlabel("temperature")
plt.ylabel("PWM")
plt.show()"""

"""plt.scatter(fan_speed ,temperatures)
slope, intercept, r_value, p_value, std_err = linregress(fan_speed,temperatures )
#plt.scatter(temperatures,fan_speed)
plt.plot(fan_speed, slope*fan_speed + intercept, color='red')
plt.title("fan_speed vs times")
plt.xlabel("fan_speed")
plt.ylabel("temperatures")
plt.show()"""
# 輸出溫度資料
#print(temperatures)

