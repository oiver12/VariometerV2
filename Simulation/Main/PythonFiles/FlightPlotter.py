import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("GPSLOG.txt")
df.head()

BBox = (df.longitude.min(),   df.longitude.max(),      
         df.latitude.min(), df.latitude.max())

# # print max/min latitude and longitude values
# BBox = (18.7, 19.2,      
#         -33.5, -32.6)

ruh_m = plt.imread('Background.jpg')
fig, ax = plt.subplots(figsize = (8,7))
ax.scatter(df.longitude, df.latitude, zorder=1, alpha= 0.5, c='w', s=10)
ax.set_title('Plotting Spatial Data')
ax.set_xlim(BBox[0],BBox[1])
ax.set_ylim(BBox[2],BBox[3])
img = ax.imshow(ruh_m, zorder=0, extent = BBox, aspect= 'equal')
plt.show()
