# Overview
Simple server to dump Epics Channel Access data to an HDF5 file.
The server gets an http callback from the Broker whenever there was an acquisition.

The format of the request is as follows:
```
{
	'range': {
		'startPulseId': 100, 
	 	'endPulseId': 120
	}, 
		
	'parameters': {
		'general/created': 'test', 
		'general/user': 'tester', 
		'general/process': 'test_process', 
		'general/instrument': 'mac', 
		'output_file': '/bla/test.h5'}
}

```

Right now this server needs to run on the same server than the

# Testing

```bash
curl -XPUT -d '{"range":{"startPulseId": 7281433214, "endPulseId": 7281489688}, "parameters":{"output_file":"test.h5"}}' http://localhost:10200/notify
``` 