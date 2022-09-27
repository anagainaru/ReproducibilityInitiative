## High-throughput Computing on HPC: a Case Study of Medical Image Processing Applications


<img src="https://github.com/MASILab/SLANTbrainSeg/blob/master/screenshot/test_volume_result.jpg" width="320px" align="left" />

Link to the application executable: [https://github.com/MASILab/SLANTbrainSeg](https://github.com/MASILab/SLANTbrainSeg)

The application performs multiple independent 3D fully convolutional network for high-resolution whole brain segmentation. 

**Input**: a MRI imagine obtained by measuring spin–lattice relaxation times of tissues

SLANT has a GPU and a CPU version as well as different versions depending on whether the network tiles are overlapped or not.

In this repo SLANT is modified to run on multiple GPUs or to run multiple MRIs concurrently on a single GPU (target
space is covered by 3 × 3 × 3 = 27 3D fully convolutional network).

# Running SLANT on multiple GPUs

# Running SLANT on a single GPU analyzing multiple MRIs concurrently


