FROM centos:centos7

ARG branch="master"

RUN yum -y install centos-release-scl epel-release
RUN yum -y update
RUN yum -y install devtoolset-9 git cmake3 mpich-devel wget zeromq-devel

ENV PATH="/usr/lib64/mpich/bin:${PATH}"
ENV LD_LIBRARY_PATH="/usr/lib64/mpich/lib:${LD_LIBRARY_PATH}"

SHELL ["scl", "enable", "devtoolset-9"]

RUN wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.tar.gz && \
    tar -xzf hdf5-1.12.0.tar.gz
WORKDIR /hdf5-1.12.0
RUN ./configure --enable-parallel && make install
RUN ln -v -s `pwd`/hdf5/lib/* /usr/lib64/ && \
    ln -v -s `pwd`/hdf5/include/* /usr/include/ && \
    ln -v -s /usr/include/mpich-x86_64/* /usr/include/

WORKDIR /
RUN git clone --depth 1 --branch ${branch} https://github.com/paulscherrerinstitute/sf_daq_buffer.git
RUN mkdir /sf_daq_buffer/build
WORKDIR /sf_daq_buffer/build
RUN cmake3 -DBUILD_JF_LIVE_WRITER=ON .. && make jf-live-writer
