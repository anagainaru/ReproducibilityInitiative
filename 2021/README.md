# Data staging for coupled simulation workflows

### Table of contents:
1. <a href="#Sec1"> Description of the system </a>
1. <a href="#Sec2"> Description of the codes and applications used </a>
2. <a href="#Sec3"> Description of the used input data </a>

<h1 id="Sec1">
1. Description of the system
</h1>

All experiments have been done on the Summit supercomputer at ORNL. 
Summit contains two IBM POWER9 processors, each with 22 SIMD Multi-Core, each capable of supporting up to 4 hardware threads.
The nodes used for our experiments contain 512 GB of DDR4 memory for use by the POWER9 processors and 1.6TB of non-volatile memory that can be used as burst buffers.
For most experiments we use 24 processes per node. A detailed description of the system architecture can be found [here](https://www.olcf.ornl.gov/summit/)

All the batch scripts used for submitting the jobs on Summit are in each application's individual folder.

Unless otherwise specified, the default versions for all the software used:
```
GCC --version
gcc (GCC) 6.4.0
CMAKE version
cmake/3.18.2 
```

<h1 id="Sec2">
1. Description of the software and applications used
</h1>

Experiments are made using simulations using the codes in `simulation` and two applications:
- **XGC** Gyrokinetic Particle Simulation of Edge Plasma
- **Gray-Scott** reaction diffusion model

### Gray-Scott reaction diffusion model

<img src="img/gray-scott.png" width="320px" align="right" />

The code can be found in the `Gray-Scott` folder. It represents a 3D 7-point stencil code to simulate the following [Gray-Scott
reaction diffusion model](https://doi.org/10.1126/science.261.5118.189):
```
u_t = Du * (u_xx + u_yy + u_zz) - u * v^2 + F * (1 - u)
v_t = Dv * (v_xx + v_yy + v_zz) + u * v^2 - (F + k) * v
```
Basis from the code can be found at [https://github.com/pnorbert/adiosvm/tree/master/Tutorial/gray-scott](https://github.com/pnorbert/adiosvm/tree/master/Tutorial/gray-scott)



### Simulations

