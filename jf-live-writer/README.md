# jf-live-writer

The jf-live-writer is packaged as a Docker container for development and 
testing.

## Building
In order to build this executable you need to specify the cmake variable
```
cmake3 -DBUILD_JF_LIVE_WRITER=ON 
```
The project will not build if you do not have installed the PHDF5 library.
Please follow instructions below on how to do that manually.

## Install PHDF5 manually 
```
wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.tar.gz
tar -xzf hdf5-1.12.0.tar.gz
cd hdf5-1.10.7
./configure --enable-parallel 
make install
sudo ln -v -s `pwd`/hdf5/lib/* /usr/lib64/
sudo ln -v -s `pwd`/hdf5/include/* /usr/include/
```

