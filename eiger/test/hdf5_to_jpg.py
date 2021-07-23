import glob
import os
import h5py
import numpy as np
from PIL import Image
import cv2


list_of_files = glob.glob('/home/hax_l/tests/*.h5') 
latest_file = max(list_of_files, key=os.path.getctime)
print("FILE: ", latest_file)
hdf = h5py.File(latest_file,'r')
array = hdf["cSAXS.EG01V01/data"]




img = array[0,:,:].astype('uint16')

img = Image.fromarray(img).convert('RGB')
img.save("export_image.jpg", "JPEG")
img.show()