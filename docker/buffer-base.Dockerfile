FROM centos:centos7

RUN yum -y install centos-release-scl epel-release && \
    yum -y update && \
    yum -y install devtoolset-9 git cmake3 mpich-devel wget zeromq-devel vim redis

ENV PATH="/usr/lib64/mpich/bin:${PATH}"
ENV LD_LIBRARY_PATH="/usr/lib64/mpich/lib:${LD_LIBRARY_PATH}"
SHELL ["scl", "enable", "devtoolset-9"]

RUN wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.tar.gz && \
    tar -xzf hdf5-1.12.0.tar.gz && \
    cd /hdf5-1.12.0 && \
    ./configure --enable-parallel && make install && \
    ln -v -s `pwd`/hdf5/lib/* /usr/lib64/ && \
    ln -v -s `pwd`/hdf5/include/* /usr/include/ && \
    ln -v -s /usr/include/mpich-x86_64/* /usr/include/
