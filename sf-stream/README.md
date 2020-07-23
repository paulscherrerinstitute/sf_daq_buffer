# sf-stream
sf-stream is the component that receives a live stream of frame data from 
sf-buffers over ZMQ and assembles them into images. This images are then 
sent again over ZMQ to external components. There is always only 1 sf-stream 
per detector.

It currently has 3 output streams:

- **Full data full metadata** rate stream (send all images and metadata)
- **Reduced data full metadata** rate stream (send less images, but 
all metadata)
- **Pulse_id** stream (send only the current pulse_id)

In addition to receiving and assembling images, sf-stream also calculates 
additional metadata and constructs the structures needed to send data in 
Array 1.0 protocol.

This component does not guarantee that the streams will always contain all 
the data - it can happen that frame resynchronization is needed, and in this 
case 1 or more frames could potentially be lost. This happens so rarely that in 
practice is not a problem. 

## Overview

![image_stream_overview](../docs/sf_daq_buffer-overview-stream.jpg)

sf-stream is a single threaded application (without counting the ZMQ IO threads)
that is used for providing live assembled images to anyone willing to listen. 

In addition, it also provides a pulse_id stream, which is the most immediate 
pulse_id feedback we currently have in case we need to synchronize external 
components to the current machine pulse_id.

## ZMQ receiving
Each ZMQ stream is coming from a separate sf-buffer. This means that we have as 
many connections as we have modules in a detector.

Messages are multipart (2 parts) and are received in PUB/SUB mode.

There is no need for special synchronization between modules as we expect that 
frames will always be in the correct order and all modules will provide the 
same frame more or less at the same time. If any of this 2 conditions is not 
met, the detector is not working properly and we cannot guaranty that sf-stream 
will work correctly.

Nonetheless we provide the capability to synchronize the streams in image 
assembly phase - this is needed rarely, but occasionally happens. In this sort 
of hiccups we usually loose only a couple of consecutive images.

### Messages format
Each message is composed by 2 parts:

- Serialization of ModuleFrame in the first part.
- Frame data in the second part.

Module frame is defined as:
```c++
#pragma pack(push)
#pragma pack(1)
struct ModuleFrame {
    uint64_t pulse_id;
    uint64_t frame_index;
    uint64_t daq_rec;
    uint64_t n_recv_packets;
    uint64_t module_id;
};
#pragma pack(pop)
```

The frame data is a 1MB (1024*512 pixels * 2 bytes/pixel) blob of data in 
**uint16** representing the detector image.

## Image assembly
We first synchronize the modules. We do this by reading all sockets and 
deciding the largest frame pulse_id among them (max_pulse_id). We then calculate 
the diff between a specific socket pulse_id and the max_pulse_id. 
This difference tells us how many messages we need to discard from a specific socket.

This discarding is the source of possible missing images in the output stream.
It can happen in 3 cases:

- At least one of the detector modules did not sent any packets for the specific 
pulse_id.
- All the packets from a specific module for a pulse_id were lost before UDP 
receiving them.
- ZMQ HWM was reached (either on the sf-buffer or sf-stream) and the message was 
dropped.

All this 3 cases are highly unlikely, so synchronization is mostly needed when 
first starting sf-stream. Different sockets connect to sf-buffers at different 
times. Apart from the initial synchronization there should be no need to 
re-synchronize modules in a healthy running environment.

If an image is missing any ZMQ messages from sf-buffers (not all modules data 
arrived), the image will be dropped. We do not do partial reconstruction in 
sf-stream. However, it is important to note, that this does not cover the case 
where frames are incomplete (missing UDP packets on sf-buffer) - we still 
assemble this images as long as at least 1 packet/frame for a specific pulse_id 
arrived.

## ZMQ sending

### Full data full metadata stream

### Reduced data full metadata stream

### Pulse_id stream