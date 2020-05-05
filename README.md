# sf_daq_buffer

Prof of concept for the SF DAQ detector needs.

## Useful links

- Hyperslab selection 
https://support.hdfgroup.org/HDF5/Tutor/phypecont.html
- Intro to lock free programming
https://preshing.com/20120612/an-introduction-to-lock-free-programming/
- POSIX compliant write order test on GPFS
https://svn.hdfgroup.org/hdf5/branches/hdf5_1_10_0/test/POSIX_Order_Write_Test_Report.pdf
- Best Practice Guide - Parallel I/O
https://prace-ri.eu/wp-content/uploads/Best-Practice-Guide_Parallel-IO.pdf
- MPI-IO/GPFS, an Optimized Implementationof MPI-IO on top of GPFS
https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1592834
- JSON library benchmarks
https://github.com/miloyip/nativejson-benchmark
- Tuning CPU scheduler for reducing latency
https://www.scylladb.com/2016/06/10/read-latency-and-scylla-jmx-process/
- Low latency 10Gbps Ethernet
https://blog.cloudflare.com/how-to-achieve-low-latency/



## Build

In order to compile you will need to install:
- devtoolset-8
- cmake3
- zeromq-devel
- hdf5-devel

Procedure:
- mkdir build
- cd build
- cmake3 ..
- make