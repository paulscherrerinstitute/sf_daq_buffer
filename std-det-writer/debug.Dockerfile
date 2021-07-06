FROM paulscherrerinstitute/sf-daq_phdf5:1.0.0

COPY . /sf_daq_buffer/

RUN mkdir /sf_daq_buffer/build && \
    cd /sf_daq_buffer/build && \
    cmake3 .. && \
    make std-det-writer

WORKDIR /sf_daq_buffer/build
