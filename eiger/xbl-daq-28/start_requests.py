from datetime import datetime
import requests
import time
import json
  
# writer agent endpoint
URL = "http://127.0.0.1:5000"
# type of write request
SYNC = "/write_sync"
ASYNC = "/write_async"
KILL = "/write_kill"
# details of request
n_images = 10
n_acquisitions = 1
headers = {'Content-type': 'application/json'}

print("Performing sync aquisitions...")
for i in range(0,n_acquisitions):
    output_file ='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/eiger_sync_%s_%s.h5' % (datetime.now().strftime("%H%M%S"), i)
    data = {'sources':'eiger', 'n_images':n_images, 'output_file':output_file}
    print("REQUEST: ", i)
    print("DATA: ", data)
    
    r = requests.post(url = "http://127.0.0.1:5000/write_sync", json=data, headers=headers)
    print("RESPONSE FROM REQUEST: ", r.text)
    data = None
    time.sleep(1)

# time.sleep(3)
# print("Performing async aquisitions...")
# for i in range(0,n_acquisitions):
#     output_file ='/home/hax_l/tests/eiger_async_%s_%s.h5' % (datetime.now().strftime("%H%M%S"), i)
#     data = {'sources':'eiger', 'n_images':n_images, 'output_file':output_file}
#     print("REQUEST: ", i)
#     print("DATA: ", data)
    
#     r = requests.post(url = "http://127.0.0.1:5000/write_async", json=data, headers=headers)
#     data = None
#     time.sleep(1)


# #//TODO  print("Testing kill aquisitions...")
