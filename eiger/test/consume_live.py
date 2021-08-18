from _ctypes import Structure
from ctypes import c_uint64, c_uint16
import zmq


image_metadata_dtype_mapping = {
    1: 'uint8',
    2: 'uint16',
    4: 'uint32',
    8: 'uint64',
    12: 'float16',
    14: 'float32',
    18: 'float64'
}

class ImageMetadata(Structure):
    _pack_ = 1
    _fields_ = [
                ("version", c_uint64),
                ("id", c_uint64),
                ("height", c_uint64),
                ("width", c_uint64),

                ("dtype", c_uint16),
                ("encoding", c_uint16),
                ("source_id", c_uint16),
                ("status", c_uint16),

                ("user_1", c_uint64),
                ("user_2", c_uint64),]

    def as_dict(self):
        return dict((f, getattr(self, f)) for f, _ in self._fields_)

flags=0
zmq_context = zmq.Context(io_threads=4)
poller = zmq.Poller()
backend_socket = zmq_context.socket(zmq.SUB)
backend_socket.setsockopt_string(zmq.SUBSCRIBE, "")
backend_socket.connect("ipc:///tmp/std-daq-cSAXS.EG01V01-image_stream")
poller.register(backend_socket, zmq.POLLIN)
while True:
    events = dict(poller.poll(2000))
    if backend_socket in events:
        metadata = ImageMetadata.from_buffer_copy(backend_socket.recv(flags))
        image = backend_socket.recv(flags, copy=False, track=False)
        print(f"Recv img id: {metadata.id}")
    else:
        print("nothing")