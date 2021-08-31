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

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(os.getcwd())))
sys.path.append(BASE_DIR+"/params/src/")
import summarize_results_functions as FC

    
hydro_fabrics_input_dir=BASE_DIR+"/params/data/hydrofabrics/releases/beta/01a/"
Resolution=30
method=1
input_twi=BASE_DIR+"/params/data/hydrofabrics/releases/beta/01a/TWI_30m/TOPMODEL_cat_file/"
outputfolder_summary=BASE_DIR+"/params/data/plots/"
if not os.path.exists(outputfolder_summary): os.mkdir(outputfolder_summary)

input_giuh=BASE_DIR+"/params/data/hydrofabrics/releases/beta/01a/GIUH_30m_1/CFE_config_file/"
outputfolder_summary=BASE_DIR+"/params/data/plots/"
if not os.path.exists(outputfolder_summary): os.mkdir(outputfolder_summary)

catchments = os.path.join(hydro_fabrics_input_dir, 'catchments.geojson')

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
FC.plot_twi(IDs,input_twi,outputfolder_summary,filename,50)
FC.plot_width_function(IDs,input_twi,outputfolder_summary,filename,2000)
FC.plot_giuh(IDs,input_giuh,outputfolder_summary,filename,15)