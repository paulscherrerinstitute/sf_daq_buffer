# jf-assembler
jf-assembler is the component that receives the confirmation from the std-udp-sync that the frames from an image are synchronized and have been arrived, and, therefore, are ready to be consumed. It fetches the frame's data and metadata and assembles them into an image on the image buffer. Once an image is assembled and ready to be consumed, it sends over ZMQ to external components. 

## Overview

jf-assembler is used for assembling the images and send to anyone willing to listen. It receives two arguments:

- detector's configuration file: detector config file path. Which includes, for example, the total number of modules from the detector that is going to be assembled;
- bit depth: defines properties of the data and the image that will be assembled. For example, number of bytes per line and bytes per image;

Such properties are defined in combination with the detector's header file, which contains the dimensions of the modules and gap pixels (if any).


## ZMQ receiving
The ZMQ receiving stream, that comes from the std-udp-sync, contains the current image_id or pulse_id that should be assembled into an image. The receiving ZMQ stream points to the buffer containing all the frames and the assembled image will be place in the image buffer.

Messages are received in PUB/SUB mode.

### Messages format
Each message is composed by one image_id/pulse_id of the next image to be assembled.

## Image assembly

With the image_id/pulse_id, the jf-assembler fetches, from the frame buffer, the source data/metadata and the destination, from the image buffer, that the assembled image should be placed.

For example, part of the detector's header file that is used in the assembler is represented below: 

```c++
#define BYTES_PER_PACKET 4144
#define DATA_BYTES_PER_PACKET 4096
#define MODULE_X_SIZE 512
#define MODULE_Y_SIZE 256
#define MODULE_N_PIXELS 131072
```

## ZMQ sending

After the image is assembled, a serialization of the ImageMetadata is sent out via ZMQ to listeners.

ImageMetadata is defined as:

```c++
struct ImageMetadata {
    uint64_t version;
    uint64_t id;
    uint64_t height;
    uint64_t width;
    uint16_t dtype;
    uint16_t encoding;
    uint16_t source_id;
    uint16_t status;
    uint64_t user_1;
    uint64_t user_2;
};
```

Messages are sent in PUB/SUB mode.


## Assembler statistics

jf-assembler logs statistics regarding the number of processed/corrupted images in the following InfluxDB format, as follows:

```bash
jf_assembler,detector_name=cSAXS.EG01V01 n_processed_images=75i,n_corrupted_images=0i,n_sync_lost_images=0i,repetition_rate=7i 1627041094036574745
```