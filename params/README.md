
# Project Title

**Description**:  
Calculates the topographic wetness index (TWI) and width function for Topmodel (version 0)
This code:
1) Download data from http://web.corral.tacc.utexas.edu/nfiedata/HAND/ for the desired HUC06 of interest
2) Allows the use of 10 or 30 meters DEM
3) Calculate a raster with the Topmodel topographic wetness index (TWI) 
4) For each sub-basin in the HUC06, calculate the TWI histogram (maximum 30 classes)
5) Calculates a raster with the distance to the outlet
6) For each sub-basin in the HUC06, calculate the Width Function with 500 meters increament to a maximum of 2500 meteres 
7) Generates the subcat.dat file needed to run topmodel - the file name contains the ID of the sub-basin. 

# Dependencies

 This code was tested in linux

# Software Requirements:
1) TauDEM (which requires gdal, mpiexec,... see https://github.com/dtarb/TauDEM)
2) Python if the TWI histogram per basin will be created. Anaconda distribution was used but is not a requirement. The following libraries are used in python: 
 	- osgeo (gdal,ogr)
 	- numpy
 	- agparse
 	- pandas
 	
3) Curl to download the HAND DEM data

## Usage
1) Edit the workflow_hand_twi_giuh.env file. This file contains all parameters for the runs, including HUC06, path to the hydrofabrics, and environmental variabels for TAUDEM and gdal
2) Run: source workflow_hand_twi_giuh.sh 

# Data Requirements:
hydrofabrics if the TWI histogram per basin will be created
The HUC06 of the area covered by the watershed
If the polygons in the hydrofabric covers multiple HUC06, all HUC06 can be specified in the "workflow_hand_twi_giuh.env" file  

## Open source licensing info


## Credits and references


