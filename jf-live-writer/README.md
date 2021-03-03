# jf-live-writer

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

