# -*- coding: utf-8 -*-
"""
Created on Fri Feb 26 15:45:46 2021

@author: lcunha
"""
# -*- coding: utf-8 -*-
"""
Created on Thu Feb 18 14:58:36 2021

@author: lcunha
"""
import os
from osgeo import ogr
from osgeo.gdalconst import GA_ReadOnly
import matplotlib.pyplot as plt     
import sys 
import pandas as pd
sys.path.append("/home/west/git_repositories/twi/workflow/")
import summarize_results_functions as FC


outputfolder_summary="/home/west/Projects/hydrofabrics/20210511/summary/"
if not os.path.exists(outputfolder_summary): os.mkdir(outputfolder_summary)
    
hydro_fabrics_input_dir="/home/west/Projects/hydrofabrics/20210511/"
Resolution=30
method=1
outputfolder_twi="/home/west/Projects/hydrofabrics/20210511/TWI_"+str(Resolution)+"m/TOPMODEL_cat_file/"
outputfolder_giuh="/home/west/Projects/hydrofabrics/20210511/GIUH_"+str(Resolution)+"m_"+str(method)+"/CFE_config_file/"

catchments = os.path.join(hydro_fabrics_input_dir, 'catchments_proj.geojson')
flowpaths = os.path.join(hydro_fabrics_input_dir, 'flowpaths.geojson')
vds = ogr.Open(catchments, GA_ReadOnly)  # TODO maybe open update if we want to write stats
assert(vds)
vlyr = vds.GetLayer(0)
total = vlyr.GetFeatureCount(force = 0)
vlyr.ResetReading()
feat = vlyr.GetNextFeature()
IDs=[]
while feat is not None:
    IDs.append(feat.GetField('ID'))
    rvds = None
    mem_ds = None
    feat = vlyr.GetNextFeature()   

filename="All"
FC.plot_twi(IDs,outputfolder_twi,outputfolder_summary,filename,50)
FC.plot_width_function(IDs,outputfolder_twi,outputfolder_summary,filename,2000)
FC.plot_giuh(IDs,outputfolder_giuh,outputfolder_summary,filename,15)