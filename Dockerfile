FROM paulscherrerinstitute/std-daq-buffer-base:1.0.4

COPY . /std_daq_buffer/

RUN mkdir /std_daq_buffer/build && \
    cd /std_daq_buffer/build && \
    # Build the project
    cmake3 .. -DUSE_EIGER=1 && \
    make && \
    # Deploy the test config.
    cp /std_daq_buffer/docker/example_detector.json . && \
    # Deploy the container starter scripts.
    cp /std_daq_buffer/docker/docker-entrypoint.sh /usr/bin/docker-entrypoint.sh  && \
    cp /std_daq_buffer/docker/redis_status.sh /usr/bin/redis_status.sh

WORKDIR /std_daq_buffer/build

ENTRYPOINT ["/usr/bin/docker-entrypoint.sh"]
CMD ["bash"]
