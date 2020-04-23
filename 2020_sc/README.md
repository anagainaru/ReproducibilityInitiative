Scripts for running the Spatially Localized Atlas Network Tiles (SLANT) application with checkpointing enabled.

## SLANT
<img src="https://github.com/MASILab/SLANTbrainSeg/blob/master/screenshot/test_volume_result.jpg" width="320px" align="left" />

Link to the application executable: [https://github.com/MASILab/SLANTbrainSeg](https://github.com/MASILab/SLANTbrainSeg)

The application performs multiple independent 3D fully convolutional network for high-resolution whole brain segmentation. 

**Input**: a MRI imagine obtained by measuring spin–lattice relaxation times of tissues

SLANT has a GPU and a CPU version as well as different versions depending on whether the network tiles are overlapped or not.
In this repo you will find instructions on how to enable checkpointing for SLANT-27, the CPU version, in which the target
space is covered by 3 × 3 × 3 = 27 3D fully convolutional network. 

## Checkpoint tool

<img src="https://criu.org/w/images/1/1c/CRIU.svg" width="180px" align="right"/>

We used the CRIU external library to perform system level checkpointing of the Docker container
without changing the code of SLANT. With each execution of SLANT we are running a daemon in charge of triggering
checkpoints at given times.

Link to the source code for CRIU: [https://github.com/checkpoint-restore/criu
](https://github.com/checkpoint-restore/criu)

Intruction on how to install CRIU can be found here:  [https://criu.org/Installation
](https://criu.org/Installation)


# Execution details

Software version:
```
$ uname -a
Linux PadmaLab 5.0.0-23-generic #24~18.04.1-Ubuntu SMP Mon Jul 29 16:12:28 UTC 2019 x86_64 x86_64 x86_64 GNU/Linux
$ sudo criu --version
Version: 3.13
$ sudo docker --version
Docker version 19.03.6, build 369ce74a3c
```

> **REMARK** If using a kernel for Ubuntu 18.04 higher than 5.0.0-32
> `echo '{ "experimental": true, "storage-driver": "devicemapper" }' > /etc/docker/daemon.json`
> `service docker restart`

The CPU version of SLANT can have a memory footprint of up to 50 GB.

> **REMARK** To change the default 1GB checkpoint size limit in CRIU when using Docker
> the DEFAULT_GHOST_LIMIT variable needs to be updated in the `criu/criu/include/cr_options.h`
> `#define DEFAULT_GHOST_LIMIT	(1 << 26) // 64G`

For testing, we run SLANT on 312 different inputs. These inputs are extracted from OASIS-3 [https://www.oasis-brains.org/](https://www.oasis-brains.org/)
and Dartmouth Raiders Dataset (DRD) datasets [https://github.com/HaxbyLab/raiders_data](https://github.com/HaxbyLab/raiders_data).

**Machine configuration**

 We run the application on a Haswell
platform composed of a server with two Intel Xeon E5-2680v3
processors (12 core @ 2,5 GHz) and 100GB main memory.

## Performance details
The application is divided into three main phases: i)
a preprocessing phase that performs transformations on the
target image (MRI is a non-scaled imaging technique) ii) deep-
learning phase iii) a post-processing phase doing label fusion
to generate final application result. Each of the tasks may
present run-to-run variation in their walltime.