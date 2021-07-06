import matplotlib.pyplot as plt
import numpy as np
plt.ion()


# These are more or less standard dacs, if you see strange effects
# you might need to tune them 
dacs = np.array([   0, 2480, 2900, 1400, 4000, 2556,  898,  848,    0,  855, 1100,
       1100,  982,  895, 2000, 1550,  570,    0], dtype=np.int32)

tb = np.zeros((256,1024), dtype = np.int32)

# Create the desired pattern 
for row in range(0,256,64):
    for col in range(0,1024,64):
        print(f'{row}, {col}')
        tb[row:row+32, col:col+32] = 63


fig, ax = plt.subplots()
im = ax.imshow(tb)


with open('trimbits.sn000', 'wb') as f:
    dacs.tofile(f)
    tb.tofile(f)