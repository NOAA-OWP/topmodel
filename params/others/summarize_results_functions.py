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

import matplotlib.pyplot as plt     
import pandas as pd
import os

    
def plot_twi(IDs,outputfolder_twi,outputfolder_summary,filename,xlim):
    plt.figure()
    ncolsAr=[];catAr=[]
    NoFiles=[]
    for cat in IDs:     
        DatFile=os.path.join(outputfolder_twi,"cat-"+str(cat)+".dat")
        catAr.append(cat)
        DatFile=DatFile.replace("cat-cat","cat")
        if os.path.exists(DatFile): 
            TWI=pd.read_csv(DatFile, sep=' ',skiprows=3,skipfooter=3,header=None,engine='python')    
            TWI=TWI.rename(columns={TWI.columns[0]: "Freq",TWI.columns[1]: "TWI"})
            ncolsAr.append(len(TWI))
            TWI=TWI.sort_values(by=['TWI'], ascending=True)
            TWI['AccumFreq']=TWI["Freq"].cumsum()
            plt.plot(TWI['TWI'],TWI['AccumFreq'],'-')
            
        else:
            NoFiles.append(cat)
            ncolsAr.append(-9)
            #print ("File does not exist " + str(cat))
    
    plt.xlabel('TWI')
    plt.ylabel('CDF')
    plt.xlim((0,xlim))
    plt.savefig(outputfolder_summary+filename+"twi.png",bbox_inches='tight')
    plt.close() 
    check_twi_data=pd.DataFrame({'cat':catAr, 'ncolsAr':ncolsAr})
   
    check_twi_data.to_csv(outputfolder_summary+filename+"twi_nclasses.txt")

def plot_width_function(IDs,outputfolder_twi,outputfolder_summary,filename,xlim):
# Read width function

    ncolsAr=[];catAr=[]
    NoFiles=[]
    for cat in IDs:            
        DatFile=os.path.join(outputfolder_twi,"cat-"+str(cat)+".dat")
        catAr.append(cat)
        DatFile=DatFile.replace("cat-cat","cat")
        if os.path.exists(DatFile): 
            f = open(DatFile, "r")
            lines = list(f)
            WFline=lines[len(lines)-2].split(" ")
            WF=[];CDF=[]
            for i in range(0,len(WFline)-1):
                 if(i % 2 == 0):
                     CDF.append(float(WFline[i]))
                 else:
                     WF.append(float(WFline[i]))
                
    
            WF_df=pd.DataFrame({'WF_ordinates':CDF, 'dist_m':WF})       
            plt.plot(WF_df['dist_m'],WF_df['WF_ordinates'],'-')
            ncolsAr.append(len(WF_df))
        else:
            NoFiles.append(cat)
            ncolsAr.append(-9)
            #print ("File does not exist " + str(cat))
    
    plt.xlabel('Distance to outlet (m)')
    plt.ylabel('CDF')
    plt.xlim((0,xlim))
    plt.savefig(outputfolder_summary+filename+"WF.png",bbox_inches='tight')
    plt.close() 
    check_WF_data=pd.DataFrame({'cat':catAr, 'ncolsAr':ncolsAr})
       
    check_WF_data.to_csv(outputfolder_summary+filename+"WF_nclasses.txt")

def plot_giuh(IDs,outputfolder_giuh,outputfolder_summary,filename,xlim):


    plt.figure()
    ncolsAr=[];catAr=[];giuhAr=[]
    NoFiles=[]
    for cat in IDs:              
        DatFile=os.path.join(outputfolder_giuh,"cat-"+str(cat)+"_bmi_config_cfe_pass.txt")
        catAr.append(cat)
        DatFile=DatFile.replace("cat-cat","cat")

        if os.path.exists(DatFile): 

            #print (DatFile)
            # TWI=pd.read_csv(DatFile, sep=' ',skiprows=3,skipfooter=3,header=None,engine='python')    
            # TWI=TWI.rename(columns={TWI.columns[0]: "Freq",TWI.columns[1]: "TWI"})
            #ncolsAr.append(len(TWI))
            # TWI=TWI.sort_values(by=['TWI'], ascending=True)
            # TWI['AccumFreq']=TWI["Freq"].cumsum()
            
            with open(DatFile) as f:
                for line in f:
                    pass
                last_line = line
            f.close()
            giuh=last_line.split("giuh_ordinates=")[1].replace("\n","").split(",")
            
            
            giuh=[float(i) for i in giuh]
            giuh_df=pd.DataFrame({'giuh_ordinates':giuh, 'time_hours':range(1,len(giuh)+1)})
            giuh_df['AccumFreq']=giuh_df["giuh_ordinates"].cumsum()
            if(max(giuh_df['AccumFreq'])>1):
                print (" Larger than 1 : " + str(cat) + " Value " + str(max(giuh_df['AccumFreq'])))
            
            plt.plot(giuh_df['time_hours'],giuh_df['AccumFreq'],'-')
            giuhAr.append(giuh)
            ncolsAr.append(len(giuh))
        else:
            NoFiles.append(cat)
            ncolsAr.append(-9)
    
    plt.xlabel('Travel time (hours)')
    plt.ylabel('CDF')
    plt.xlim((0,xlim))
    plt.savefig(outputfolder_summary+filename+"giuh.png",bbox_inches='tight')
    plt.close() 
    check_giuh_data=pd.DataFrame({'cat':catAr, 'ncolsAr':ncolsAr})
   
    check_giuh_data.to_csv(outputfolder_summary+filename+"giuh_nclasses.txt")