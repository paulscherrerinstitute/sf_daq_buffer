from bottle import route, run, request, abort
import json

import data_api

import logging
logger = logging.getLogger("logger");

# This is how the notification look like
# {
#     'range': {
#         'startPulseId': 100,
#         'endPulseId': 120
#     },
#
#     'parameters': {
#         'general/created': 'test',
#         'general/user': 'tester',
#         'general/process': 'test_process',
#         'general/instrument': 'mac',
#         'output_file': '/bla/test.h5'}    # this is usually the full path
# }

channel_list = ["S10-CPCL-VM1MGC:LOAD"]  # specified channel is only for test purposes


@route('/notify', method='PUT')
def put_document():
    data = request.body.read()
    if not data:
        abort(400, 'No data received')

    try:
        download_data(json.loads(data))
    except Exception as e:
        logger.warning("Download data failed", e)


def download_data(config):

    logger.info("Dump data to hdf5 ...")
    # logger.info(config)

    start_pulse = config["range"]["startPulseId"]
    end_pulse = config["range"]["endPulseId"]

    start_date, end_date = data_api.get_global_date([start_pulse, end_pulse])

    # append _CA to the filename
    filename = config["parameters"]["output_file"]
    new_filename = filename[:-3]+"_CA"+filename[-3:]

    logger.info("Retrieving data for interval start: " + str(start_date) + " end: " + str(end_date))
    data = data_api.get_data(channel_list, start=start_date, end= end_date)
    logger.info("Persist data to hdf5 file")
    data_api.to_hdf5(data, new_filename, overwrite=True, compression=None, shuffle=False)


def read_channels(filename):
    with open(filename) as f:
        lines = f.readlines()

    channels = []
    for line in lines:
        line = line.strip()
        if line: # if not empty line
            channels.append(line) # remove all leading and trailing spaces

    return channels


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Channel Access archiver dump to hdf5')
    parser.add_argument('--channels', dest='channel_list', default="tests/channels.txt", help='channels to dump')

    args = parser.parse_args()
    print(args.channel_list)

    global channel_list
    channel_list = read_channels(args.channel_list)
    logger.info("Using channel list: " + " ".join(channel_list))

    run(host='localhost', port=10200)


if __name__ == '__main__':
    main()
