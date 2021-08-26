"""

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
import json
gdal.PushErrorHandler('CPLQuietErrorHandler')
ogr.UseExceptions()

def create_weight_raster(FileName,matrixBasin,ncol, nrow,geotransform,ProjSys):
    from osgeo import gdal
    import numpy as np
#    import BasicFunction as BF  
    from osgeo import gdal, osr
    from osgeo import ogr              
        # Extract data block
    driver = gdal.GetDriverByName('GTiff')

    dst_ds = driver.Create( FileName, int(ncol), int(nrow), int(1), gdal.GDT_Float64 )    
    #print (geotransform)
    dst_ds.SetGeoTransform( geotransform )  
    # if(len(ProjSys)>1):  
    dst_ds.SetProjection( ProjSys )
    # else:
    #     srs = osr.SpatialReference()
    #     srs.SetWellKnownGeogCS( 'NAD83' )
    #     dst_ds.SetProjection(srs)
    
    
    dst_ds.GetRasterBand(1).WriteArray( matrixBasin )

    dst_ds = None    


def CLipRaster(src_filename,match_filename,dst_filename):
    from osgeo import gdal, gdalconst
    
    # Source
    
    src = gdal.Open(src_filename, gdalconst.GA_ReadOnly)

    assert src, "Could not open " + src_filename
    src_proj = src.GetProjection()
    #src_geotrans = src.GetGeoTransform()
    
    # We want a section of source that matches this:
    
    match_ds = gdal.Open(match_filename, gdalconst.GA_ReadOnly)
    assert match_ds, "Could not open " + match_filename
    match_proj = match_ds.GetProjection()
    match_geotrans = match_ds.GetGeoTransform()
    wide = match_ds.RasterXSize
    high = match_ds.RasterYSize
    
    # Output / destination
    
    dst = gdal.GetDriverByName('GTiff').Create(dst_filename, wide, high, 1, gdalconst.GDT_Float32)
    dst.SetGeoTransform( match_geotrans )
    dst.SetProjection( match_proj)
    
    # Do the work
    gdal.ReprojectImage(src, dst, src_proj, match_proj, gdalconst.GRA_Bilinear)

    return

def ContantVelocityMS(namestr,taudem_folder,hydrofabrics_folder,channel,overland,gully):
    
    ad8_file=taudem_folder+namestr+"ad8.tif"
    rds = gdal.Open(ad8_file, GA_ReadOnly)
    assert rds, "Could not open " + ad8_file
    rb = rds.GetRasterBand(1) 
    ncol=rds.RasterXSize 
    nrow=rds.RasterYSize
    geotransform = rds.GetGeoTransform()
    ProjSys=rds.GetProjection()
    ad8_array = rb.ReadAsArray()
    
    src_file=taudem_folder+namestr+"src.tif"
    rds = gdal.Open(src_file, GA_ReadOnly)
    assert rds, "Could not open " + src_file
    rb = rds.GetRasterBand(1)
    rgt = rds.GetGeoTransform()
    src_array = rb.ReadAsArray()    
    
    inv_vel_matrix=np.zeros((nrow,ncol))+overland
    inv_vel_matrix[ad8_array>2]=gully
    inv_vel_matrix[src_array==1]=channel
    inv_vel_matrix=1.0/(inv_vel_matrix*60) # This is the time matrix, but used the same variable to save space
    
    wg_file=taudem_folder+namestr+"wg1.tif"
    create_weight_raster(wg_file,inv_vel_matrix,ncol, nrow,geotransform,ProjSys)

    return 1

def SDTTVelocityMS(namestr,taudem_folder,hydrofabrics_folder,rain_intensity_mm_h):  

    #From Wong, 1995
    #The units for equations (11) and (12) are min for tc, m m"1 for S, m2 s"1 for qu,mm h"1 for i, and m for L.
    import math as m

    ad8_file=taudem_folder+namestr+"ad8.tif"
    rds = gdal.Open(ad8_file, GA_ReadOnly)
    assert rds, "Could not open " + ad8_file
    rb = rds.GetRasterBand(1) 
    ncol=rds.RasterXSize 
    nrow=rds.RasterYSize
    geotransform = rds.GetGeoTransform()
    ProjSys=rds.GetProjection()
    ad8_array = rb.ReadAsArray()
    ad8_array[ad8_array<0]=0

    manning_file=taudem_folder+namestr+"man.tif"
    rds = gdal.Open(manning_file, GA_ReadOnly)
    assert rds, "Could not open " + manning_file
    rb = rds.GetRasterBand(1)
    rgt = rds.GetGeoTransform()
    manning_array = rb.ReadAsArray()  
    manning_array[manning_array<=0]=0.001
    
    slope_file=taudem_folder+namestr+"slp.tif"
    rds = gdal.Open(slope_file, GA_ReadOnly)
    assert rds, "Could not open " + slope_file
    rb = rds.GetRasterBand(1)
    rgt = rds.GetGeoTransform()
    slope_array = rb.ReadAsArray()  
    slope_array[slope_array<=0]=0.001

    Travel_time=7.0*np.power((manning_array*1.0)/(np.power(slope_array,0.5)),0.6)*m.pow(rain_intensity_mm_h,-0.4)* (np.power((ad8_array+1),0.6)-np.power(ad8_array,0.6))
    
    wg_file=taudem_folder+namestr+"wg2.tif"
    create_weight_raster(wg_file,Travel_time,ncol, nrow,geotransform,ProjSys)

    return 1

def SDTTVelocityMS_channel(namestr,taudem_folder,hydrofabrics_folder,rain_intensity_mm_h,channel):  
    #From Wong, 1995
    #The units for equations (11) and (12) are min for tc, m m"1 for S, m2 s"1 for qu,mm h"1 for i, and m for L.
    import math as m

    ad8_file=taudem_folder+namestr+"ad8.tif"
    rds = gdal.Open(ad8_file, GA_ReadOnly)
    assert rds, "Could not open " + ad8_file
    rb = rds.GetRasterBand(1) 
    ncol=rds.RasterXSize 
    nrow=rds.RasterYSize
    geotransform = rds.GetGeoTransform()
    ProjSys=rds.GetProjection()
    ad8_array = rb.ReadAsArray()
    ad8_array=ad8_array+1
    ad8_array[ad8_array<0]=0
    
    manning_file=taudem_folder+namestr+"man.tif"
    rds = gdal.Open(manning_file, GA_ReadOnly)
    assert rds, "Could not open " + manning_file
    rb = rds.GetRasterBand(1)
    rgt = rds.GetGeoTransform()
    manning_array = rb.ReadAsArray()  
    manning_array[manning_array<=0]=0.01    
    
    slope_file=taudem_folder+namestr+"slp.tif"
    rds = gdal.Open(slope_file, GA_ReadOnly)
    assert rds, "Could not open " + slope_file
    rb = rds.GetRasterBand(1)
    rgt = rds.GetGeoTransform()
    slope_array = rb.ReadAsArray()  
    slope_array[slope_array<=0]=0.001

    Travel_hillslope=7.0*np.power((manning_array*1.0)/(np.power(slope_array,0.5)),0.6)*m.pow(rain_intensity_mm_h,-0.4)* (np.power((ad8_array+1),0.6)-np.power(ad8_array,0.6))     
    Travel_time=np.where(grid.riv==1, Travel_river, Travel_hillslope)
    
    wg_file=taudem_folder+namestr+"wg3.tif"
    create_weight_raster(wg_file,Travel_time,ncol, nrow,geotransform,ProjSys)
    
    return 1
    


def generate_travel_time_by_pixel(namestr,
                                  taudem_folder,
                                  hydrofabrics_folder,
                                  manning="",
                                  rain_intensity_mm_h=10.0,
                                  resolution=10,
                                  method=1,
                                  channel=1.0,
                                  overland=0.5,
                                  gully=0.2):
    
    clipped_manning=taudem_folder+namestr+"man.tif"
    if(method>1):
        print (manning)
        print (clipped_manning)
        print (taudem_folder+namestr+"ad8.tif")
        CLipRaster(manning,taudem_folder+namestr+"ad8.tif",clipped_manning) 

    flag=1
    if(method==1): flag=ContantVelocityMS(namestr,taudem_folder,hydrofabrics_folder,channel,overland,gully)
    elif(method==2): flag=SDTTVelocityMS(namestr,taudem_folder,hydrofabrics_folder,rain_intensity_mm_h)
    elif(method==3): flag=SDTTVelocityMS_Chanell(taudem_folder,hydrofabrics_folder,rain_intensity_mm_h,channel)
    else:  
        flag=0
        print ("wrong option for method, chose 1, 2, or 3")

    return flag


if __name__ == "__main__":

                                       
    parser = argparse.ArgumentParser()

    parser.add_argument("namestr",
                        help="Name of the Taudem files")
    parser.add_argument("taudem_folder",
                        help="Taudem Folder")
    parser.add_argument("hydrofabrics_folder",
                        help="Hydrofabrics older")    
    parser.add_argument("--manning", type=str, default='', metavar="manning",
                        help="Manning for overland flow - required for the SDTT method")         
    parser.add_argument("--rain_intensity_mm_h", type=float, default=10., metavar="rainrate",
                        help="Rain intensity required for the SDTT method")
    parser.add_argument("--resolution", type=int, default=10, metavar="resolution",
                        help="DEM resolution")
    parser.add_argument("--method", type=int, default=1, metavar="method",
                        help="Method used to generate pixel time")
    parser.add_argument("--channel", type=float, default=1.0, metavar="channel",
                        help="Constant velocity in the channel ")
    parser.add_argument("--overland", type=float, default=0.5, metavar="overland",
                        help="Constant velocity for overland flow")    
    parser.add_argument("--gully", type=float, default=0.2, metavar="gully",
                        help="Constant velocity for gully flow")    
    args = parser.parse_args()

    generate_travel_time_by_pixel(args.namestr,args.taudem_folder,args.hydrofabrics_folder,
                                  manning=args.manning,
                                  rain_intensity_mm_h=args.rain_intensity_mm_h,
                                  resolution=args.resolution,
                                  method=args.method,
                                  channel=args.channel,
                                  overland=args.overland,
                                  gully=args.gully)
