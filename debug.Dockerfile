FROM paulscherrerinstitute/sf-daq_phdf5:1.0.1

COPY . /sf_daq_buffer/

RUN mkdir /sf_daq_buffer/build && \
    cd /sf_daq_buffer/build && \
    # Build the project
    cmake3 .. -DUSE_EIGER=1 && \
    make && \
    # Deploy the test config.
    cp /sf_daq_buffer/docker/example_detector.json .

WORKDIR /sf_daq_buffer/build
