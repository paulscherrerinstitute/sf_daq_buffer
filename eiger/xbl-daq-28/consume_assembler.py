import zmq
from _ctypes import Structure
from ctypes import c_uint64, c_uint16

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
                ("user_2", c_uint64)]

    def as_dict(self):
        return dict((f, getattr(self, f)) for f, _ in self._fields_)





zmq_context = zmq.Context()
backend_socket = zmq_context.socket(zmq.SUB)
#backend_socket.setsockopt(zmq.RCVTIMEO, 100)
backend_socket.setsockopt_string(zmq.SUBSCRIBE, "")
backend_socket.connect("ipc:///tmp/std-daq-cSAXS.EG01V01-assembler")

while True:
    image_bytes = backend_socket.recv()
    image_meta = ImageMetadata.from_buffer_copy(image_bytes)
    print(f'Image received: {image_meta.id}')
