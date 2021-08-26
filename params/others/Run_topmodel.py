#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Aug 13 12:51:06 2021

@author: west
"""

import subprocess
import os
import glob
import shutil
import pandas as pd
import matplotlib.pyplot as plt   
import sys  
sys.path.append("/home/west/git_repositories/twi/workflow/")
import summarize_results_functions as FC

outputfolder_results="/home/west/git_repositories/topmodel_fork_NOAA/topmodel/params/data/TopModel_Results/"
if not os.path.exists(outputfolder_results): os.mkdir(outputfolder_results)

run_flag=1
process_results_flag=1
Resolution=30    
#hydro_fabrics_input_dir="/home/west/Projects/hydrofabrics/20210511/"
#outputfolder_twi="/home/west/Projects/hydrofabrics/20210511/TWI_"+str(Resolution)+"m/TOPMODEL_cat_file/"
outputfolder_twi="/home/west/git_repositories/topmodel_fork_NOAA/topmodel/params/data/TOPMODEL_cat_file/"
topmodel_folder="/home/west/git_repositories/topmodel/"

topmodel="/home/west/git_repositories/topmodel/run_bmi"
cat_file_new=topmodel_folder+"/data/subcat.dat"
out_hyd_file=topmodel_folder+"hyd.out"
out_topmod_file=topmodel_folder+"topmod.out"
list_of_files=glob.glob(outputfolder_twi+"*")


hyd_df=pd.DataFrame()
#Reference output

obs_df=pd.read_csv(topmodel_folder+"hyd_ori.out", delimiter = " ",header=None,index_col=0)[1].to_frame()
obs_df=obs_df.rename(columns={1: "Obs"})

hyd_ref=pd.read_csv(topmodel_folder+"hyd_ori.out", delimiter = " ",header=None,index_col=0)[2].to_frame()
hyd_ref=hyd_ref.rename(columns={2: "Ref"})

for ifile in range(0,len(list_of_files)):

    cat_file=list_of_files[ifile]
    cat_basename=os.path.basename(cat_file).replace(".dat","") 
    basin=cat_basename.replace("cat-","")
    out_hyd_file_new=outputfolder_results+cat_basename+"_hyd.out"  
    out_topmod_file_new=outputfolder_results+cat_basename+"_topmod.out"
    if(run_flag):    
        # Copy cat file
        cat_file=list_of_files[ifile]
        cat_basename=os.path.basename(cat_file).replace(".dat","") 
        shutil.copy(cat_file, cat_file_new)
        
        # remove outputs    
    
        # Run topmodel     
        p = subprocess.Popen(topmodel, stdout=subprocess.PIPE)
        output = p.communicate()[0]
        print(output)
    
        # Copy outputs                  
        shutil.copy(out_hyd_file, out_hyd_file_new)            
        shutil.copy(out_topmod_file, out_topmod_file_new)
        
        os.remove(out_hyd_file) 
        os.remove(out_topmod_file) 
    if(process_results_flag):  
        hyd=pd.read_csv(out_hyd_file_new, delimiter = " ",header=None,index_col=0)[2].to_frame()
        hyd=hyd.rename(columns={2: basin})
        hyd_df = pd.concat([hyd_df, hyd], axis=1)
        
        
plt.figure()   
plt.plot(hyd_df)
plt.plot(hyd_ref,'b')
plt.plot(obs_df,'r')
plt.xlabel('Time(hours)')
plt.ylabel('Runoff(m)')
plt.savefig(outputfolder_results+"hyd.png",bbox_inches='tight')
plt.close() 

nelem=len(hyd_df.columns)
Runoff_peak=hyd_df.max().sort_values()
Runoff_peak.to_csv(outputfolder_results+"Runoff_peak.csv")
Runoff_volume=hyd_df.sum().sort_values()
Runoff_volume.to_csv(outputfolder_results+"Runoff_volume.csv")
n_to_plot=10#int(nelem/20)
#plot TWI and GIUH for lower and higher 10th percentile
filename="HighPeak"
outputfolder_summary="/home/west/Projects/hydrofabrics/20210511/summary/"
IDs_high=Runoff_peak.iloc[nelem-n_to_plot:nelem].index
FC.plot_twi(IDs_high,outputfolder_twi,outputfolder_summary,filename,50)
FC.plot_width_function(IDs_high,outputfolder_twi,outputfolder_summary,filename,2000)

filename="LowPeak"
IDs_low=Runoff_peak.iloc[0:n_to_plot].index
FC.plot_twi(IDs_low,outputfolder_twi,outputfolder_summary,filename,50)
FC.plot_width_function(IDs_low,outputfolder_twi,outputfolder_summary,filename,2000)

filename="AboveTh"
IDs_thr=Runoff_peak[Runoff_peak>0.001].index
FC.plot_twi(IDs_thr,outputfolder_twi,outputfolder_summary,filename,50)
FC.plot_width_function(IDs_thr,outputfolder_twi,outputfolder_summary,filename,2000)

    # Read results
    