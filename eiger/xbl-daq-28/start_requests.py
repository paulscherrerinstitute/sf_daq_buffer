#!/usr/bin/python

from datetime import datetime
import sys
import getopt
import requests
import time
import json
from urllib.parse import urljoin

URL = "http://127.0.0.1:5000"
SYNC = "/write_sync"
ASYNC = "/write_async"
DET = "/detector/eiger"

headers = {'Content-type':'application/json'}

def prepare_det(dr):
    print("Stoping the detector...")
    stop_data = {'cmd': "STOP"}
    r = requests.post(url=urljoin(URL,DET), headers=headers, json=stop_data)
    time.sleep(0.1)
    print("Configuring the detector...")
    data_config = {"det_name":"eiger","config":{"frames":5000, "triggers":1, "exptime":0.0005, "period": 0.001,  "timing":"auto", "tengiga":1, "dr":dr}}
    r = requests.post(url=urljoin(URL,DET), headers=headers, json=data_config)
    time.sleep(1)
    print("Starting the detector...")
    start_data = {'cmd':"START"}
    r = requests.post(url = urljoin(URL,DET), headers=headers, json=start_data)




def main(argv):

    # details of request
    n_images = 5
    n_acquisitions = 5
    
    for dr in [8, 16, 32]:
        prepare_det(dr)
        for j in [SYNC, ASYNC]:
            print(f'Performing { j } aquisitions (bit depth { dr })...')
            time.sleep(1.5)
            for i in range(0,n_acquisitions):
                output_file ='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/output_folder%s_%s_req%s_dr%s.h5' % (j, datetime.now().strftime("%H%M%S"), i, dr)
                data = {'sources':'BEC.EG01V01', 'n_images':n_images, 'output_file':output_file}
                r = requests.post(url = urljoin(URL,SYNC), json=data, headers=headers)
                print("RESPONSE FROM REQUEST: ", r.text)





if __name__ == "__main__":
    main(sys.argv[1:])
