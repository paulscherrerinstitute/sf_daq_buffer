#!/usr/bin/python

from datetime import datetime
import sys
import getopt
import requests
import time
import json


def main(argv):

    # writer agent endpoint
    URL = "http://127.0.0.1:5000"
    # details of request
    n_images = 10
    n_acquisitions = 10
    headers = {'Content-type': 'application/json'}




    print("Configuring the detector...")
    data_config = {"det_name":"eiger","config":{"frames":36000,"dr":16, "triggers":1, "exptime":0.0005, "period": 0.001,  "timing":"auto", "tengiga":1}}
    r = requests.post(url="http://127.0.0.1:5000/detector/eiger", headers=headers, json=data_config)
    time.sleep(0.5)


    print("Starting the detector...")
    start_data = {'cmd':"START"}
    r = requests.post(url = "http://127.0.0.1:5000/detector/eiger", headers=headers, json=start_data)
    time.sleep(0.5)

    print("Performing sync aquisitions...")
    for i in range(0,n_acquisitions):
        output_file ='/home/dbe/git/sf_daq_buffer/eiger/xbl-daq-28/output_folder/eiger_sync_%s_%s.h5' % (datetime.now().strftime("%H%M%S"), i)
        #output_file='/tmp/output.h5'
        data = {'sources':'BEC.EG01V01', 'n_images':n_images, 'output_file':output_file}
        print("REQUEST: ", i)
        print("DATA: ", data)

        r = requests.post(url = "http://127.0.0.1:5000/write_async", json=data, headers=headers)
        print("RESPONSE FROM REQUEST: ", r.text)
        time.sleep(0.2)

if __name__ == "__main__":
    main(sys.argv[1:])
