"""
Zonal Statistics
Vector-Raster Analysis
Modified by Luciana Cunha from 2013 Matthew Perry and AsgerPetersen:

usage: generate_twi_per_basin.py [-h] [--output flag [--buffer distance] [--nodata value] [-f FORMAT]
                      catchments twi_raster slope_raster outputfolder_twi
positional arguments:
  namest                HUC Number
  catchments            hydrofabrics catchment
  twi_raster            Twi Raster file - generated with workflow_hand_twi_giuh.sh
  slope_raster          Slope Raster file - generated with workflow_hand_twi_giuh.sh
  dist_to_outlet_raster          Distance to Outlet Raster file - generated with workflow_hand_twi_giuh.sh
  outputfolder_twi      Output folder

optional arguments:
  -h, --help            show this help message and exit
  --output flag
  --buffer distance     Buffer geometry by this distance before calculation
  --nodata value        Use this nodata value instead of value from raster
  --preload             Preload entire raster into memory instead of a read
                        per vector feature  
"""
from osgeo import gdal, ogr
from osgeo.gdalconst import *
import numpy as np
import sys
import argparse
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib
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
    y1 = int((bbox[3] - originY) / pixel_height)
    y2 = int((bbox[2] - originY) / pixel_height) 

     
    xsize = x2 - x1
    ysize = y2 - y1
    return (x1, y1, xsize, ysize)



def generate_twi_per_basin(namestr,catchments, twi_raster,slope_raster, dist_to_outlet_raster,outputfolder_twi,
                    output_flag=1,
                    nodata_value=None,
                    global_src_extent=False,
                    buffer_distance=0):


    outputfolder_twi_param_file=outputfolder_twi+"/TOPMODEL_param/"
    if not os.path.exists(outputfolder_twi_param_file): os.mkdir(outputfolder_twi_param_file)
    if(output_flag==1): 
        outputfolder_twi_config_file=outputfolder_twi+"/TOPMODEL_cat_file/"
        if not os.path.exists(outputfolder_twi_config_file): os.mkdir(outputfolder_twi_config_file)
        
    rds = gdal.Open(twi_raster, GA_ReadOnly)
    assert rds, "Could not open twi raster"
    twi = rds.GetRasterBand(1) #previous rb
    rgt = rds.GetGeoTransform()
    
    rds2 = gdal.Open(slope_raster, GA_ReadOnly)
    assert rds2, "Could not open slope raster"
    slope = rds2.GetRasterBand(1) #previous rb2
    rgt2 = rds2.GetGeoTransform()

    rds3 = gdal.Open(dist_to_outlet_raster, GA_ReadOnly)
    assert rds3, "Could not open distance to stream raster"
    d2s = rds3.GetRasterBand(1)
    rgt3 = rds3.GetGeoTransform()
    
    if nodata_value:
        # Override with user specified nodata
        nodata_value = float(nodata_value)
        twi.SetNoDataValue(nodata_value)
    else:
        # Use nodata from band
        nodata_value = float(twi.GetNoDataValue())
    

    # Warn if nodata is NaN as this will not work with the mask (as NaN != NaN)
    assert nodata_value == nodata_value, "Cannot handle NaN nodata value"

    if buffer_distance:
        buffer_distance = float(buffer_distance)


    vds = ogr.Open(catchments, GA_ReadOnly) 
    assert(vds)
    vlyr =  vds.GetLayer(0)
    #vdefn = vlyr.GetLayerDefn()

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
        src_array = twi.ReadAsArray(*src_offset)
        src_array_slope = slope.ReadAsArray(*src_offset)
        src_array_dist = d2s.ReadAsArray(*src_offset)
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

    # Loop through vectors
      
    skippednulgeoms = False
    total = vlyr.GetFeatureCount(force = 0)
    vlyr.ResetReading()
    count = 0
    feat = vlyr.GetNextFeature()
    CatIDdict={}
    Test=-999
    # while flag==0:
    #     cat = feat.GetField('ID')
    #     count = count + 1
        
    #     if(cat==Test):
    #         flag=1
    #     else:
    #         rvds = None
    #         mem_ds = None
    #         feat = vlyr.GetNextFeature()

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
            src_array = twi.ReadAsArray(*src_offset)
            src_array_slope = slope.ReadAsArray(*src_offset)
            src_array_dist = d2s.ReadAsArray(*src_offset)            
            # calculate new geotransform of the feature subset
            new_gt = (
                (rgt[0] + (src_offset[0] * rgt[1])),
                rgt[1],
                0.0,
                (rgt[3] + (src_offset[1] * rgt[5])),
                0.0,
                rgt[5]
            )
            

        #print ("src_array")   
        #print (src_array)
        # Create a temporary vector layer in memory
        if not src_array is None:
            
            # 1 - First calculate histogram TWI
            mem_ds = mem_drv.CreateDataSource('out')
            mem_layer = mem_ds.CreateLayer('mem_lyr', None, mem_type)
            mem_layer.CreateFeature(mem_feat)
    
            # Rasterize river network 
            rvds = driver.Create('', src_offset[2], src_offset[3], 1, gdal.GDT_Byte)
            rvds.SetGeoTransform(new_gt)
            gdal.RasterizeLayer(rvds, [1], mem_layer, burn_values=[1])
            rv_array = rvds.ReadAsArray()     
            
            # Define large TWI for slope = 0 
            src_array[src_array_slope==0.0]=50.
            src_array[(np.isinf(src_array))]=50.
            # Mask the source data array with our current feature
            # we take the logical_not to flip 0<->1 to get the correct mask effect
            # we also mask out nodata values explictly
            
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
               
                maskedArray=np.ma.filled(masked.astype(float), np.nan).flatten()
                
                #N_elem_valid=(masked>0).sum()
                #Porc_valid=100*N_elem_valid/N_elem_polygon
                #print (" Col " + str(len(src_array)) + " row " +str(len(src_array[0])) + " row " + str(len(src_array[0])))
                #all_values1=len(src_array)*len(src_array[0])
                if(cat==Test): ("col " + str(len(masked))  + " row " + str(len(masked[0])))
                if(cat==Test): 
                    print ("maskedArray " + str(len(maskedArray)))
    
                if(cat==Test): 
                    print (" all_values1 " + str(all_values1) + " all_values " +str(all_values))
                maskedArray2=maskedArray[(maskedArray!=nodata_value) & (~np.isnan(maskedArray)) & (maskedArray>0)]
                if(cat==Test): 
                    print ("maskedArray2 " + str(len(maskedArray2)))
                    print (str(maskedArray))
                    print (str(maskedArray2))
                sorted_array = np.sort(maskedArray2)
                all_values=((masked.mask==False) & (masked.data>0)).sum() # False where the basin is, not sure why. So counting pixels in the basin
                filtered_values=len(sorted_array) # Valid values
            
                                                
                if(cat==Test): print ("In loop to generate info" + str(cat) + " Freq "+ str(100*filtered_values/all_values) + " all values "+ str(all_values) + " filtered_values " + str(filtered_values))
                #print ("In loop to generate info" + str(cat) + " Freq "+ str(100*filtered_values/all_values) + " all values "+ str(all_values) + " filtered_values " + str(filtered_values))
                LessThan50=sorted_array[sorted_array<50]
                if(len(LessThan50)>0): 
                    MaxValue=max(LessThan50)
                    sorted_array[sorted_array>MaxValue]=MaxValue
                nclasses=min(len(np.unique(sorted_array)),30)
                TWI = pd.DataFrame(columns=['TWI'], data=sorted_array)
                hist=np.histogram(TWI['TWI'].values, bins=nclasses)
        
                CDF=pd.DataFrame({'Nelem':hist[0].T, 'TWI':hist[1][1:].T}).sort_values(by=['TWI'], ascending=False)
                CDF['Freq']=CDF['Nelem']/sum(CDF['Nelem'])
                CDF['AccumFreq']=CDF['Freq'].cumsum()
                #Scatter plot of CDF
                #CDF.to_csv(os.path.join(outputfolder_twi, "CDF_" + str(cat) + '.csv'), index=False)
                #CDFplot = CDF.plot(kind='scatter',x='TWI',y='AccumFreq',color='blue').get_figure()
                DatFile=os.path.join(outputfolder_twi_param_file,"cat-"+str(cat)+"_twi.csv")
                CDF.to_csv(DatFile)     
                
                # 2 - Second calculate the width function
    
                # Mask the source data array with our current feature
                # we take the logical_not to flip 0<->1 to get the correct mask effect
                # we also mask out nodata values explictly
                masked = np.ma.MaskedArray(
                    src_array_dist,
                    mask=np.logical_or(
                        src_array == nodata_value,
                        np.logical_not(rv_array) # remove this since it was creating issues with 1
                    )
                )
                #N_elem_valid=(masked>0).sum()
                #Porc_valid=100*N_elem_valid/N_elem_polygon
                #print (" Col " + str(len(src_array)) + " row " +str(len(src_array[0])) + " row " + str(len(src_array[0])))
                all_values1=len(src_array_dist)*len(src_array_dist[0])
                maskedArray=np.ma.filled(masked.astype(float), np.nan).flatten()
    
                all_values=len(maskedArray)
                maskedArray2=maskedArray[(maskedArray!=nodata_value) & (~np.isnan(maskedArray)) & (maskedArray>0)]
    
                sorted_array = np.sort(maskedArray2)                 
                if(len(np.unique(sorted_array))>5): 
                    Per5=np.percentile(sorted_array,10)
                    Per95=np.percentile(sorted_array,95)
                    sorted_array=sorted_array[(sorted_array>=Per5) & (sorted_array<=Per95)]
                    sorted_array=(sorted_array-min(sorted_array))
                else:
                    sorted_array=np.zeros(20)+500.
                # These values are hardcoded now due to the problem of catchment boundary created by using 
                # different DEMS to generate the hydrofabrics, and in this analysis
                # also, the hydrofabric polygones are modify which can also create a problem
                max_class=min(2501,max(sorted_array))
                bins=np.arange(0,max_class,500)
                nclasses_width_function=len(bins)
                if(len(bins)<5): 
                    bins=5    
                    nclasses_width_function=5
                
                dist_to_outlet = pd.DataFrame(columns=['dist_to_outlet'], data=sorted_array)
                
                hist=np.histogram(dist_to_outlet['dist_to_outlet'].values, bins=bins)

                CDF_D2O=pd.DataFrame({'Nelem':hist[0].T, 'dist_to_outlet':hist[1][1:].T}).sort_values(by=['dist_to_outlet'], ascending=True)
                CDF_D2O['Freq']=CDF_D2O['Nelem']/sum(CDF_D2O['Nelem'])
                CDF_D2O['AccumFreq']=CDF_D2O['Freq'].cumsum()
                #Scatter plot of CDF
                #CDF.to_csv(os.path.join(outputfolder_twi, "CDF_" + str(cat) + '.csv'), index=False)
                #CDFplot = CDF.plot(kind='scatter',x='TWI',y='AccumFreq',color='blue').get_figure()
                DatFile=os.path.join(outputfolder_twi_param_file,"cat-"+str(cat)+"_d2o.csv")
                DatFile=DatFile.replace("cat-cat-","cat-")
                CDF_D2O.to_csv(DatFile)                  
                
                if(output_flag==1):
                #DirCat=os.path.join(outputfolder_twi, str(cat))
                #if not os.path.exists(DirCat): os.mkdir(DirCat)
                    DatFile=os.path.join(outputfolder_twi_config_file,"cat-"+str(cat)+".dat")
                    DatFile=DatFile.replace("cat-cat-","cat-")
                    f= open(DatFile, "w")
                    f.write("1  1  1\n")
                    f.write("%s" %("Extracted study basin: " + str(cat) +"\n"))
                    f.write("%s" %(str(nclasses)+" 1\n"))
                    for icdf in range(0,len(CDF)):
                        strdata="{0:.6f}".format((round(CDF['Freq'].iloc[icdf],6))) + " " + "{0:.6f}".format((round(CDF['TWI'].iloc[icdf],6))) +"\n"
                        f.write("%s" %(strdata))
                    f.write(str(nclasses_width_function)+"\n") 
                    strdata="0.0 0.1 "
                    for icdf in range(0,len(CDF_D2O)):
                        strdata=strdata+"{0:.6f}".format((round(CDF_D2O['AccumFreq'].iloc[icdf],6))) + " " + "{0:.6f}".format((round(CDF_D2O['dist_to_outlet'].iloc[icdf],6))) +" "
                    strdata=strdata+"\n"
                    f.write("%s" %(strdata))
                    #f.write("0.0  500.  0.5  1000.  1.0  1500.\n") 
                    f.write("$mapfile.dat\n") 
                    f.close()
                   
                
                # Catdatadict={}
                # Catdatadict['Freq'] = CDF['Freq'].values.tolist()
                # Catdatadict['TWI'] = CDF['TWI'].values.tolist()
                # Catdatadict['Freq_Width_Function'] = CDF_D2O['AccumFreq'].values.tolist()
                # Catdatadict['Width_Function'] = CDF_D2O['dist_to_outlet'].values.tolist()                
                # # cat ID
                # CatIDdict[str(cat)] = Catdatadict
        
    
        rvds = None
        mem_ds = None
        feat = vlyr.GetNextFeature()

    # with open(os.path.join(outputfolder_twi,namestr+'TWI.json'), 'w') as outfile:
    #         json.dump(CatIDdict, outfile)
            
    vds = None
    rds = None
    rds2 = None
    rds3 = None
    return CatIDdict


if __name__ == "__main__":


    parser = argparse.ArgumentParser()

    parser.add_argument("namest",
                        help="HUC name")
    parser.add_argument("catchments",
                        help="Vector source")
    parser.add_argument("twi_raster",
                        help="TWI Raster file")
    parser.add_argument("slope_raster",
                        help=" Slope file")
    parser.add_argument("dist_to_outlet_raster",
                        help=" distance to outlet")
    
    
    parser.add_argument("outputfolder_twi",
                        help="Output folder")
    parser.add_argument("--buffer", type=float, default=0, metavar="distance",
                        help="Buffer geometry by this distance before calculation")
    parser.add_argument("--nodata", type=float, default=None, metavar="value",
                        help="Use this nodata value instead of value from raster")
    parser.add_argument("--output", type=int, default=1, metavar="flag",
                        help="Write topmodel subcat files ")
    parser.add_argument("--preload", action="store_true",
                        help="Preload entire raster into memory instead of a read per vector feature")

    args = parser.parse_args()

    generate_twi_per_basin(args.namest,args.catchments, args.twi_raster, args.slope_raster, args.dist_to_outlet_raster, args.outputfolder_twi,
                    nodata_value = args.nodata,
                    global_src_extent = args.preload,
                    buffer_distance = args.buffer,
                    output_flag = args.output,                    
                    )


   