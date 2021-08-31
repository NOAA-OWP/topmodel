"""
Zonal Statistics
Vector-Raster Analysis
Modified by Luciana Cunha from 2013 Matthew Perry and AsgerPetersen:

usage: generate_twi_per_basin.py [-h] [--output flag [--buffer distance] [--nodata value] 
                      catchments twi_raster slope_raster outputfolder_twi
positional arguments:
  namest                HUC Number
  catchments            hydrofabrics catchment
  time_to_stream_raster Time to stream in minutes - generated with workflow_hand_twi_giuh.sh
  soil_params_file      CSV file with soil parameters from NWM 2.1 - part of the hydrofabric released in 08/2021
  GW_params_file        CSV file with groundwater parameters from NWM 2.1 - part of the hydrofabric released in 08/2021
  outputfolder_giuh     Output folder

optional arguments:
  -h, --help            show this help message and exit
  --output flag         0-Only generates the param file, 1- generates the cfe config file
  --buffer distance     Buffer geometry by this distance before calculation
  --nodata value        Use this nodata value instead of value from raster
  --preload             Preload entire raster into memory instead of a read
  --output_flag         Output the config file for CFE 
"""


from osgeo import gdal, ogr
from osgeo.gdalconst import *
import numpy as np
import sys
import argparse
import pandas as pd
import os
#import json
gdal.PushErrorHandler('CPLQuietErrorHandler')
ogr.UseExceptions()

def bbox_to_pixel_offsets(gt, bbox):
    originX = gt[0]
    originY = gt[3]
    pixel_width = gt[1]
    pixel_height = gt[5]
    x1 = int((bbox[0] - originX) / pixel_width)
    x2 = int((bbox[1] - originX) / pixel_width) 
    #x2 = int((bbox[1] - originX) / pixel_width) + 1
    y1 = int((bbox[3] - originY) / pixel_height)
    y2 = int((bbox[2] - originY) / pixel_height) 
    #y2 = int((bbox[2] - originY) / pixel_height) + 1
     
    xsize = x2 - x1
    ysize = y2 - y1
    return (x1, y1, xsize, ysize)



def generate_giuh_per_basin(namestr,catchments, time_to_stream_raster, soil_params_file, GW_params_file, outputfolder_giuh,
                    output_flag=1,
                    nodata_value=None,
                    global_src_extent=False,
                    buffer_distance=0.001):
    
    if not os.path.exists(catchments):  print ("does not exist "  + catchments)
    if not os.path.exists(time_to_stream_raster):  print ("does not exist "  + time_to_stream_raster)
    if not os.path.exists(soil_params_file):  print ("does not exist "  + soil_params_file)
    if not os.path.exists(GW_params_file):  print ("does not exist "  + GW_params_file)
    

    
    outputfolder_giuh_param_file=outputfolder_giuh+"/CFE_GIUH/"    
    if not os.path.exists(outputfolder_giuh_param_file): os.mkdir(outputfolder_giuh_param_file)
    if(output_flag==1): 
        outputfolder_giuh_config_file=outputfolder_giuh+"/CFE_config_file/"
        if not os.path.exists(outputfolder_giuh_config_file): os.mkdir(outputfolder_giuh_config_file)
    rds = gdal.Open(time_to_stream_raster, GA_ReadOnly)
    assert rds, "Could not open raster" +time_to_stream_raster
    rb = rds.GetRasterBand(1)
    rgt = rds.GetGeoTransform()
    
    if nodata_value:
        # Override with user specified nodata
        nodata_value = float(nodata_value)
        rb.SetNoDataValue(nodata_value)
    else:
        # Use nodata from band
        nodata_value = float(rb.GetNoDataValue())
    # Warn if nodata is NaN as this will not work with the mask (as NaN != NaN)
    assert nodata_value == nodata_value, "Cannot handle NaN nodata value"

    if buffer_distance:
        buffer_distance = float(buffer_distance)


    vds = ogr.Open(catchments, GA_ReadOnly)  
    assert(vds)
    vlyr =   vds.GetLayer(0)
#    vdefn = vlyr.GetLayerDefn()

    # Calculate (potentially buffered) vector layer extent
    vlyr_extent = vlyr.GetExtent()
    if buffer_distance:
        expand_by = [-buffer_distance, buffer_distance, -buffer_distance, buffer_distance]
        vlyr_extent = [a + b for a, b in zip(vlyr_extent, expand_by)]

    
    # create an in-memory numpy array of the source raster data
    # covering the whole extent of the vector layer
    if global_src_extent:
        # use global source extent
        # useful only when disk IO or raster scanning inefficiencies are your limiting factor
        # advantage: reads raster data in one pass
        # disadvantage: large vector extents may have big memory requirements
        src_offset = bbox_to_pixel_offsets(rgt, vlyr_extent)
        #print (str(src_offset))
        src_array = rb.ReadAsArray(*src_offset)
        
        # calculate new geotransform of the layer subset
        new_gt = (
            (rgt[0] + (src_offset[0] * rgt[1])),
            rgt[1],
            0.0,
            (rgt[3] + (src_offset[1] * rgt[5])),
            0.0,
            rgt[5]
        )

    mem_drv = ogr.GetDriverByName('Memory')
    driver = gdal.GetDriverByName('MEM')
 
        # if(output_flag==1):

        
    if(output_flag==1) & (os.path.isfile(soil_params_file)) & (os.path.isfile(soil_params_file)): 
        soil_params=pd.read_csv(soil_params_file,index_col=0)
        if(not "cat-" in str(soil_params.index[0])): soil_params.index = 'cat-' + soil_params.index.astype(str)
    
        GW_params=pd.read_csv(GW_params_file,index_col=0)
        if(not "cat-" in str(GW_params.index[0])): GW_params.index = 'cat-' + GW_params.index.astype(str)

        # Remove Nan for the parameters that are used to generate config file
        # Replace with values from the original CFE config file
        #print ("Getting values from table ")
        soil_params['bexp_soil_layers_stag=1_Time=1']= soil_params['bexp_soil_layers_stag=1_Time=1'].fillna(16)                    
        soil_params['dksat_soil_layers_stag=1_Time=1']= soil_params['dksat_soil_layers_stag=1_Time=1'].fillna(0.00000338)
        soil_params['psisat_soil_layers_stag=1_Time=1']= soil_params['psisat_soil_layers_stag=1_Time=1'].fillna(0.355)  
        soil_params['slope_Time=1']= soil_params['slope_Time=1'].replace(np.nan,1.0)
        soil_params['smcmax_soil_layers_stag=1_Time=1']= soil_params['smcmax_soil_layers_stag=1_Time=1'].fillna(0.439)
        soil_params['smcwlt_soil_layers_stag=1_Time=1']= soil_params['smcwlt_soil_layers_stag=1_Time=1'].fillna(0.066)
        GW_params['Zmax'] = GW_params['Zmax'].fillna(16.0)
        GW_params['Coeff'] = GW_params['Coeff'].fillna(0.01)
        GW_params['Expon'] = GW_params['Expon'].fillna(6.0)
    else:
        
        skippednulgeoms = False
        total = vlyr.GetFeatureCount(force = 0)
        vlyr.ResetReading()
        count = 0
        feat = vlyr.GetNextFeature()
        IDAr=[]
        while feat is not None:
            cat = feat.GetField('ID')
            IDAr.append(cat)
            feat = vlyr.GetNextFeature()        
        soil_params=pd.DataFrame(index=IDAr)
        GW_params=pd.DataFrame(index=IDAr)
        soil_params['bexp_soil_layers_stag=1_Time=1']= 16.0                    
        soil_params['dksat_soil_layers_stag=1_Time=1']= 0.00000338
        soil_params['psisat_soil_layers_stag=1_Time=1']= 0.355
        soil_params['slope_Time=1']= 1.0
        soil_params['smcmax_soil_layers_stag=1_Time=1']= 0.439
        soil_params['smcwlt_soil_layers_stag=1_Time=1']= 0.066
        GW_params['Zmax']= 16.0
        GW_params['Coeff'] = 0.01
        GW_params['Expon'] = 6.0

    # soil_params_depth=2.0;soil_params_b_st=4.05;soil_params_mult_st=1000.0;soil_params_satdk_st=0.00000338; soil_params_satpsi_st=0.355    
    # soil_params_slop_st=1.0; soil_params_smcmax_st=0.439; soil_params_wltsmc_st=0.066;
    # max_gw_storage_st=16.0; Cgw_st=0.01; expon_st=6.0;
    # gw_storage_st=50;alpha_fc_st=0.33; soil_storage_st=66.7
    # K_nash_st=0.03;K_lf_st=0.01
    # nash_storage_st='0.0,0.0'; giuh_ordinates_st='0.06,0.51,0.28,0.12,0.03'
          
    
    # if(output_flag==1):
    skippednulgeoms = False
    total = vlyr.GetFeatureCount(force = 0)
    vlyr.ResetReading()
    count = 0
    feat = vlyr.GetNextFeature()

    while feat is not None:
        cat = feat.GetField('ID')
        count = count + 1
        
        if count % 100 == 0:
            sys.stdout.write("\r{0} of {1}".format(count, total))
            sys.stdout.flush()
        if feat.GetGeometryRef() is None:
            # Null geometry. Write to dst and continue
            if not skippednulgeoms:
                print ("\nWarning: Skipping nullgeoms\n")
                skippednulgeoms = True
            feat = vlyr.GetNextFeature()
            continue
        mem_feat = feat.Clone()
        mem_type = mem_feat.GetGeometryRef().GetGeometryType()
        if buffer_distance:
            mem_type = ogr.wkbPolygon
            mem_feat.SetGeometryDirectly( mem_feat.GetGeometryRef().Buffer(buffer_distance) )

        if not global_src_extent:
            # use local source extent
            # fastest option when you have fast disks and well indexed raster (ie tiled Geotiff)
            # advantage: each feature uses the smallest raster chunk
            # disadvantage: lots of reads on the source raster
            src_offset = bbox_to_pixel_offsets(rgt, mem_feat.geometry().GetEnvelope())
            #print (str(src_offset))
            src_array = rb.ReadAsArray(*src_offset)
            
            # calculate new geotransform of the feature subset
            new_gt = (
                (rgt[0] + (src_offset[0] * rgt[1])),
                rgt[1],
                0.0,
                (rgt[3] + (src_offset[1] * rgt[5])),
                0.0,
                rgt[5]
            )
            
        if not src_array is None:
            #print ("src_array")   
            #print (src_array)
            # Create a temporary vector layer in memory
            mem_ds = mem_drv.CreateDataSource('out')
            mem_layer = mem_ds.CreateLayer('mem_lyr', None, mem_type)
            mem_layer.CreateFeature(mem_feat)
    
            # Rasterize it
            rvds = driver.Create('', src_offset[2], src_offset[3], 1, gdal.GDT_Byte)
            rvds.SetGeoTransform(new_gt)
            gdal.RasterizeLayer(rvds, [1], mem_layer, burn_values=[1])
            rv_array = rvds.ReadAsArray()

            # Mask basin area
            masked_basin = np.ma.MaskedArray(
                src_array,
                mask=np.logical_not(rv_array) 
                )
            all_values_in_basin=((masked_basin.mask==False)).sum() # False where the basin is, not sure why. So counting pixels in the basin
                
            # Also remove missing data - keep only valid values
            masked = np.ma.MaskedArray(
                src_array,
                mask=np.logical_or(
                    src_array == nodata_value,
                    np.logical_not(rv_array) # remove this since it was creating issues with 1
                )
            )
            all_valid_values_in_basin=((masked.mask==False)).sum()
            Check=100*all_valid_values_in_basin/all_values_in_basin # Porcentage of valid numbers in the polygone
            if(Check>80):   
       
                #Create a 1-d array - include nan for points outside of the polygone
                maskedArray=np.ma.filled(masked.astype(float), np.nan).flatten()
                
                #remove all values outside of the polygone which are marked as nan
                maskedArray2=maskedArray[(maskedArray!=nodata_value) & (~np.isnan(maskedArray)) & (maskedArray>=0)] # Values covered by the polygon
                # Due to incompatibilities with hydrofabrics
                
                sorted_array = np.sort(maskedArray2)    
                  
                if(len(np.unique(sorted_array))>5): 
                    Per5=np.percentile(sorted_array,10)
                    Per95=np.percentile(sorted_array,95)
                    sorted_array=sorted_array[(sorted_array>=Per5) & (sorted_array<=Per95)]
                    sorted_array=(sorted_array-min(sorted_array))
                else:
                    sorted_array=np.zeros(20)+3600.
                Per5=np.percentile(sorted_array,5)
                Per95=np.percentile(sorted_array,95)
                if(len(np.unique(sorted_array))>10): sorted_array=sorted_array[(sorted_array>=Per5) & (sorted_array<=Per95)]
                sorted_array=(sorted_array-min(sorted_array))/60.
                
                AllData = pd.DataFrame(columns=['TravelTimeHour'], data=sorted_array)                
                max_Nclasses=min(15,max(AllData['TravelTimeHour']))
                max_Nclasses=max(3,max_Nclasses)
                classes=np.arange(0,max_Nclasses, 1)
                
                hist=np.histogram(AllData['TravelTimeHour'].values, bins=classes)
                
                CDF=pd.DataFrame({'Nelem':hist[0].T, 'TravelTimeHour':hist[1][1:].T}).sort_values(by=['TravelTimeHour'], ascending=True)
                CDF['Freq']=CDF['Nelem']/sum(CDF['Nelem'])
                CDF['AccumFreq']=CDF['Freq'].cumsum()            
                DatFile=os.path.join(outputfolder_giuh_param_file,"cat-"+str(cat)+"_giuh.csv")
                DatFile=DatFile.replace("cat-cat-","cat-")
                CDF.to_csv(DatFile)       
                
                if(output_flag==1):
                    DatFile=os.path.join(outputfolder_giuh_config_file,"cat-"+str(cat)+"_bmi_config_cfe_pass.txt")
                    DatFile=DatFile.replace("cat-cat-","cat-")
                    f= open(DatFile, "w")
                    
                    f.write("%s" %("forcing_file=BMI\n"))
                    f.write("%s" %("soil_params.depth=2.0\n"))
                    f.write("%s" %("soil_params.b="+str(soil_params.loc[cat]['bexp_soil_layers_stag=1_Time=1'])+"\n"))                    
                    f.write("%s" %("soil_params.mult=1000.0\n"))
                    f.write("%s" %("soil_params.satdk="+str(soil_params.loc[cat]['dksat_soil_layers_stag=1_Time=1'])+"\n"))
                    f.write("%s" %("soil_params.satpsi="+str(soil_params.loc[cat]['psisat_soil_layers_stag=1_Time=1'])+"\n"))
                    f.write("%s" %("soil_params.slop="+str(soil_params.loc[cat]['slope_Time=1'])+"\n"))
                    f.write("%s" %("soil_params.smcmax="+str(soil_params.loc[cat]['smcmax_soil_layers_stag=1_Time=1'])+"\n"))
                    f.write("%s" %("soil_params.wltsmc="+str(soil_params.loc[cat]['smcwlt_soil_layers_stag=1_Time=1'])+"\n"))
                    f.write("%s" %("max_gw_storage="+str(GW_params.loc[cat]['Zmax'])+"\n"))
                    f.write("%s" %("Cgw="+str(GW_params.loc[cat]['Coeff'])+"\n"))
                    f.write("%s" %("expon="+str(GW_params.loc[cat]['Expon'])+"\n"))
                    f.write("%s" %("gw_storage=50%\n"))
                    f.write("%s" %("alpha_fc=0.33\n"))
                    f.write("%s" %("soil_storage=66.7%\n"))
                    f.write("%s" %("K_nash=0.03\n"))
                    f.write("%s" %("nash_storage=0.0,0.0\n"))
                    giuh="giuh_ordinates="+"{0:.2f}".format((round(CDF['Freq'].iloc[0],4)))
                    for icdf in range(1,len(CDF)):
                        giuh=giuh+","+"{0:.2f}".format((round(CDF['Freq'].iloc[icdf],4)))
                    giuh =giuh+"\n"  
                    f.write("%s" %(giuh))
                    f.close()
            # 07/30/2021 Commented because it can overwrite a good file when the selected polygon is in the raster area, but not in the HUC 
            # 
            # else:
            #     print (LU)
            #     #DatFile=os.path.join(outputfolder_giuh_param_file,"cat-"+str(cat)+"_giuh.csv")
            #     #CDF.to_csv(DatFile)
            #     if(output_flag==1):                   
            #         DatFile=os.path.join(outputfolder_giuh_config_file,"cat-"+str(cat)+"_bmi_config_cfe_pass.txt")
            #         f= open(DatFile, "w")
            #         string="forcing_file=BMI\nsoil_params.depth=2.0\nsoil_params.b=4.05\nsoil_params.mult=1000.0\nsoil_params.satdk=0.00000338\nsoil_params.satpsi=0.355\nsoil_params.slop=1.0\nsoil_params.smcmax=0.439\nsoil_params.wltsmc=0.066\nmax_gw_storage=16.0\nCgw=0.01\nexpon=6.0\ngw_storage=50%\nalpha_fc=0.33\nsoil_storage=66.7%\nK_nash=0.03\nK_lf=0.01\nnash_storage=0.0,0.0\n"
            #         f.write("%s" %(string))
            #         giuh="giuh_ordinates=1.0,0.0\n"
            #         f.write("%s" %(giuh))
            #         f.close()                   

        src_array = None
        rvds = None
        mem_ds = None
        feat = vlyr.GetNextFeature()



    vds = None
    rds = None


if __name__ == "__main__":


    parser = argparse.ArgumentParser()

    parser.add_argument("namest",
                        help="HUC name")
    parser.add_argument("catchments",
                        help="Vector source - json")
    parser.add_argument("time_to_stream_raster",
                        help="Time to stream (minutes) raster - tif file")
    parser.add_argument("soil_params_file",
                        help="CSV file with NWM 2.1 soil params")
    parser.add_argument("GW_params_file",
                        help="CSV file with NWM 2.1 groundwater params")    
    parser.add_argument("outputfolder_giuh",
                        help="Output folder")

    parser.add_argument("--buffer", type=float, default=0, metavar="distance",
                        help="Buffer geometry by this distance before calculation")
    parser.add_argument("--nodata", type=float, default=None, metavar="value",
                        help="Use this nodata value instead of value from raster")
    parser.add_argument("--output", type=int, default=1, metavar="flag",
                        help="Write CFE file containing GIUH parameters")
    parser.add_argument("--preload", action="store_true",
                        help="Preload entire raster into memory instead of a read per vector feature")


    args = parser.parse_args()
    
    generate_giuh_per_basin(args.namest,args.catchments, args.time_to_stream_raster, args.soil_params_file,args.GW_params_file,args.outputfolder_giuh,                    
                    nodata_value = args.nodata,
                    global_src_extent = args.preload,
                    buffer_distance = args.buffer,
                    output_flag = args.output,                    
                    )
