# sf_daq_buffer

Overview of current architecture and component interaction.

![Overview image](docs/sf_daq_buffer-overview.jpg)

## Useful links

### Architecture
- POSIX compliant write order test on GPFS
https://svn.hdfgroup.org/hdf5/branches/hdf5_1_10_0/test/POSIX_Order_Write_Test_Report.pdf
- Best Practice Guide - Parallel I/O
https://prace-ri.eu/wp-content/uploads/Best-Practice-Guide_Parallel-IO.pdf
- MPI-IO/GPFS, an Optimized Implementation of MPI-IO on top of GPFS
https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1592834
- 10GE network tests with UDP - European XFEL
https://indico.cern.ch/event/212228/contributions/1507212/attachments/333941/466017/10GE_network_tests_with_UDP.pdf

### Software
- Intro to lock free programming
https://preshing.com/20120612/an-introduction-to-lock-free-programming/
- JSON library benchmarks
https://github.com/miloyip/nativejson-benchmark
- Kernel bypass
https://blog.cloudflare.com/kernel-bypass/
- PACKET_MMAP
https://www.kernel.org/doc/Documentation/networking/packet_mmap.txt
- Hyperslab selection 
https://support.hdfgroup.org/HDF5/Tutor/phypecont.html
- Caching and Buffering in HDF5
https://de.slideshare.net/HDFEOS/caching-and-buffering-in-hdf5
- Chunking in HDF5
https://portal.hdfgroup.org/display/HDF5/Chunking+in+HDF5
- Setting Raw Data Chunk Cache Parameters in HDF5
https://support.hdfgroup.org/pubs/rfcs/RFC_chunk_cache_functions.pdf
- Memory model synchronization modes
https://gcc.gnu.org/wiki/Atomic/GCCMM/AtomicSync
- Is Parallel Programming Hard, And, If So, What Can You Do About It?
https://mirrors.edge.kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.2018.12.08a.pdf
- Linux kernel profiling with perf
https://perf.wiki.kernel.org/index.php/Tutorial


### Linux configuration
- CFS: Completely fair process scheduling in Linux
https://opensource.com/article/19/2/fair-scheduling-linux
- perf sched for Linux CPU scheduler analysis
http://www.brendangregg.com/blog/2017-03-16/perf-sched.html
- Tuning CPU scheduler for reducing latency
https://www.scylladb.com/2016/06/10/read-latency-and-scylla-jmx-process/
- RHEL7: How to get started with CGroups.
https://www.certdepot.net/rhel7-get-started-cgroups/
- Cpusets
https://www.kernel.org/doc/Documentation/cgroup-v1/cpusets.txt
- Understanding mlx5 ethtool Counters
https://community.mellanox.com/s/article/understanding-mlx5-ethtool-counters
- Red Hat Enterprise Linux Network Performance Tuning Guide
https://access.redhat.com/sites/default/files/attachments/20150325_network_performance_tuning.pdf
- Low latency 10Gbps Ethernet
https://blog.cloudflare.com/how-to-achieve-low-latency/
- Monitoring and Tuning the Linux Networking Stack: Receiving Data
https://blog.packagecloud.io/eng/2016/06/22/monitoring-tuning-linux-networking-stack-receiving-data/#procnetsoftnet_stat
- Making linux do hard real-time
https://www.slideshare.net/jserv/realtime-linux
- Linux timing and scheduling granularity
https://fritshoogland.wordpress.com/2018/03/13/linux-timing-and-scheduling-granularity/
- Raw Ethernet Programming: Basic Introduction - Code Example
https://community.mellanox.com/s/article/raw-ethernet-programming--basic-introduction---code-example
- Performance Tuning for Mellanox Adapters
https://community.mellanox.com/s/article/performance-tuning-for-mellanox-adapters
- UEFI Workload-based Performance and TuningGuide for HPE ProLiant Gen10 https://support.hpe.com/hpesc/public/docDisplay?docId=a00016408en_us

## Build

To compile this repo you will need to install the following packages on RH7:
- devtoolset-9
- cmake3
- zeromq-devel
- hdf5-devel

```bash
yum install devtoolset-9
yum install cmake3
yum install zeromq-devel
yum install hdf5-devel
yum install jsoncpp-devel
```

Step by step procedure to build the repo:

```bash
scl enable devtoolset-9 bash
git clone https://github.com/paulscherrerinstitute/sf_daq_buffer.git
cd sf_daq_buffer
mkdir build
cd build/
cmake3 ..
make
```

It is recommended to create symbolic links to the executables you will be using 
inside your PATH.

Example:
```bash
ln -s "$(pwd)""/""sf_buffer" /usr/bin/sf_buffer
ln -s "$(pwd)""/""sf_stream" /usr/bin/sf_stream
ln -s "$(pwd)""/""sf_replay" /usr/bin/sf_replay
ln -s "$(pwd)""/""sf_writer" /usr/bin/sf_writer
```

## Warnings

### Zeromq

Zeromq version 4.1.4 (default on RH7) has a LINGER bug. Sometimes, the last 
message is not sent (the connection gets dropped before the message is in the buffer).
Since we use PUSH/PULL to modulate the sf_replay speed, this is a key functionality we are using.

Please install a later version:
```bash
cd /etc/yum.repos.d/
wget https://download.opensuse.org/repositories/network:messaging:zeromq:release-stable/RHEL_7/network:messaging:zeromq:release-stable.repo
yum install zeromq-devel
```

## Terminology

In order to unify the way we write code and talk about concept the following 
terminology definitions should be followed:

- frame (data from a single module)
- image (data of the assembled image)
- start_pulse_id and stop_pulse_id (not end_pulse_id) is used to determine the 
inclusive range (both start and stop pulse_id are included) of pulses.