# Spin table project  
### Center for Space Engineering, Utah State University  

This repository provides code that interfaces with various sensors for the spin table. It also includes a COSMOS config folder.  

### to build, follow these steps:  
`mkdir build` to create a build directory  
`cd build` to move into that directory  
`cmake ..` to generate the makefile  
then  
`make sensor` to build the executable for the sensor pi  
`make motor` to build the executable for the motor pi  
`make` to build both  
  
make sure to run the executable with sudo 

### to set up COSMOS, follow these steps  
copy the `config` folder into a COSMOS install directory, or  
copy everything from the COSMOS install directory (minus the `config` folder) into the repository directory
