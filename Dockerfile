FROM paulscherrerinstitute/std-daq-buffer-base:1.0.4

COPY . /std_daq_buffer/

RUN mkdir /std_daq_buffer/build && \
    cd /std_daq_buffer/build && \
    # Build the project
    cmake3 .. -DUSE_EIGER=1 && \
    make && \
    # Deploy the test config.
    cp /std_daq_buffer/docker/example_detector.json .

WORKDIR /std_daq_buffer/build
