namespace stream_config
{
    // N of IO threads to receive data from modules.
    const int STREAM_ZMQ_IO_THREADS = 5;
    // How long should the RECV queue be.
    const size_t STREAM_RCVHWM = 100;
    // Size of buffer between the receiving and sending part.
    const int STREAM_FASTQUEUE_SLOTS = 5;
    // If the modules are offset more than 1000 pulses, crush.
    const uint64_t PULSE_OFFSET_LIMIT = 1000;
    // SNDHWM for live processing socket.
    const int PROCESSING_ZMQ_SNDHWM = 10;
    // Keep the last second of pulses in the buffer.
    const int PULSE_ZMQ_SNDHWM = 100;
}
