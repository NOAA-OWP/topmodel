## workflow_hand_twi_giuh: 
## Uses TauDEM to extract topmodel TWI and/or the parameters needed to generate CFE GIUH
## Data: HAND DEM available at http://web.corral.tacc.utexas.edu/nfiedata/HAND/
## version: v0
## Author: Luciana Cunha <luciana.kindl.da.cunha at noaa.gov>
## Date: 05/26/2021

# This code was tested in linux

# Requirements:
# 	TauDEM (which requires gdal, mpiexec,... see https://github.com/dtarb/TauDEM)
# 	python if the TWI histogram per basin will be created
# 	curl to download the data

# Data Requirements:
# 	hydrofabrics if the TWI histogram per basin will be created

# Use:
# 	Edit the workflow_hand_twi_giuh.env file
# 	Run source workflow_hand_twi_giuh.sh 


source ./workflow_hand_twi_giuh.env

temp="$(pwd)" 
Dir=${temp//src/}
echo ${Dir}

mkdir ${Dir}${dem_dir}
mkdir ${Dir}${out_dir_taudem}
mkdir ${Dir}${out_dir_twi}
mkdir ${Dir}${out_dir_giuh}

for val in  ${HUC[@]}; do
	hucid=$val
	if [[ "$Resolution" -eq 30 ]];then
		file_name=${hucid}_30m	
	else
		file_name=${hucid}
	fi	
	START_TIME=$(date +%s)
	echo "running ${hucid}, for ${Variable}, at ${Resolution} m resolution"
	#-----------------------------------------------
	# Download HAND DEM
	#START_TIME=$(date +%s)
	# Download HAND dataset
	echo ${Dir}${out_dir_taudem}/${hucid}/${file_name}fel.tif
	if test -f ${Dir}${out_dir_taudem}/${hucid}/${file_name}fel.tif; then
		echo  ${Dir}${out_dir_taudem}/${hucid}/${file_name}fel.tif
	else
		#echo "Check for file ${hucid}fel.tif\n"
		if test -f ${Dir}${dem_dir}/${hucid}fel.tif; then
			echo "${hucid}fel.tif exists"
		else
		 	echo "Downloading file ${hucid}fel.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}fel.tif -o ${Dir}${dem_dir}/${hucid}fel.tif
		fi
	fi
	#	END_TIME=$(date +%s)  
	
	Outdir=${Dir}${out_dir_taudem}/$hucid/
	mkdir ${Dir}${out_dir_taudem}/$hucid

	#-----------------------------------------------
	# Download shapefile with area
	if test -f ${Outdir}${hucid}-wbd.shp; then
		echo "${hucid}-wbd.shp exists"
	else
        	echo "Downloading file ${hucid}-wbd.shp"
	  	curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.dbf -o ${Outdir}${hucid}-wbd.dbf
		curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.prj -o ${Outdir}${hucid}-wbd.prj
	  	curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.shx -o ${Outdir}${hucid}-wbd.shx
		curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.shp -o ${Outdir}${hucid}-wbd.shp	
	fi
	#END_TIME6=$(date +%s)
	#echo "It took $(($END_TIME6-$END_TIME5)) seconds to dowload the data/n"  
		
	if [[ "$Resolution" -eq 30 ]];then
		
		#If resolution equal to 30, aggregate DEM and run pitremove and dinfflowdir
		#-----------------------------------------------
		# Resample DEM to 30 x 30 meters
		if test -f ${Outdir}${file_name}.tif; then
			echo "${Outdir}${file_name}.tif exists"
		else
			echo "Resampled DEM to 30 x 30 meters"
			gdalwarp -tr 0.0003086429087 0.0003086429087 -r average ${Dir}${dem_dir}/${hucid}fel.tif ${Outdir}${file_name}.tif
		fi
		FelPath=${Outdir}/${file_name}fel.tif

		#-----------------------------------------------
		# pitremove  
		echo "Process DEM"
		if test -f ${Outdir}${file_name}fel.tif; then
			echo "${Outdir}${file_name}fel.tif exists"
		else
			mpiexec -np $nproc pitremove -z ${Outdir}${file_name}.tif -fel ${FelPath}
		fi 	
		
		#-----------------------------------------------
		# dinfflowdir  
		if test -f ${Outdir}${file_name}ang.tif; then
			echo "${file_name}p.tif exists\n"
		else
			mpiexec -np $nproc dinfflowdir -ang ${Outdir}${file_name}ang.tif -slp ${Outdir}${file_name}slp.tif -fel ${FelPath}
		fi
					
	else
	
		#If resolution equal to 10, download the info from the website 
		FelPath=${Dir}${dem_dir}/${file_name}fel.tif
		#-----------------------------------------------
		# Download other available datasets
		if test -f ${Outdir}${file_name}slp.tif; then
			echo "${Outdir}${file_name}slp.tif exists"
		else
		 	echo "Downloading file ${hucid}slp.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}slp.tif -o ${Outdir}${file_name}slp.tif
		fi
				
		if test -f ${Outdir}${file_name}ang.tif; then
			echo "${file_name}ang.tif exists"
		else
		 	echo "Downloading file ${Outdir}${file_name}ang.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}ang.tif -o ${Outdir}${file_name}ang.tif
 	
		fi			
		if test -f ${Outdir}${file_name}p.tif; then
			echo "${file_name}p.tif exists"
		else
		 	echo "Downloading file ${Outdir}${file_name}p.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}p.tif -o ${Outdir}${file_name}p.tif
 	
		fi	
	fi

	#-----------------------------------------------
	# areadinf - sca is not available for the 10 meters, so we need to calculate it independently of the resolution
	if test -f ${Outdir}${file_name}sca.tif; then
		echo "${Outdir}${file_name}sca.tif exists"
	else
		mpiexec -np $nproc  areadinf -ang ${Outdir}${file_name}ang.tif -sca ${Outdir}${file_name}sca.tif 
	fi
	
	if [[ $Variable == *"TWI"* ]]; then

		#-----------------------------------------------
		# twi
		echo "Generate Topographic Wetness Index"
		if test -f ${Outdir}${file_name}twi.tif; then
			echo "${Outdir}${file_name}twi.tif exists"
		else
		mpiexec -np $nproc twi -slp ${Outdir}${file_name}slp.tif -sca ${Outdir}${file_name}sca.tif -twi ${Outdir}${file_name}twi.tif
		fi

		#-----------------------------------------------
		# Crop TWI and slope
		echo "Crop DEM to the area of interest based on Shapefile"
		if test -f ${Outdir}${file_name}twi_cr.tif; then
			echo "${Outdir}${file_name}twi_cr.tif exists"
		else
			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${Dir}${out_dir_taudem}/${hucid}/${hucid}-wbd.shp -dstalpha ${Outdir}${file_name}twi.tif -dstnodata "-999.0" ${Outdir}${file_name}twi_cr.tif
			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${Dir}${out_dir_taudem}/${hucid}/${hucid}-wbd.shp -dstalpha ${Outdir}${file_name}slp.tif -dstnodata "-999.0" ${Outdir}${file_name}slp_cr.tif
		fi

		if test -f  ${Dir}${hydrofabrics_directory}flowpaths_wgs84.json; then
			echo "catchments_wgs84.json exists"
		else
		 	echo "Reproject hydrofabrics file catchments_wgs84.json"

			ogr2ogr -f "GeoJSON" ${Dir}${hydrofabrics_directory}catchments_wgs84.json ${Dir}${hydrofabrics_directory}catchments.geojson  -s_srs EPSG:5070 -t_srs EPSG:4326
			ogr2ogr -f "GeoJSON" ${Dir}${hydrofabrics_directory}flowpaths_wgs84.json ${Dir}${hydrofabrics_directory}flowpaths.geojson -s_srs EPSG:5070 -t_srs EPSG:4326
			
		fi
		
		if test -f ${Outdir}${file_name}hf.tif; then
			echo "catchments_wgs84.geojson exists"
		else
		 	echo "rasterize flow line"
			
			gdal_translate -scale 0 40000000000000 0 0 ${Outdir}${file_name}fel.tif ${Outdir}${file_name}hf.tif	
			gdal_rasterize -b 1 -burn 1  ${Dir}${hydrofabrics_directory}flowpaths_wgs84.json ${Outdir}${file_name}hf.tif			
		fi

	fi
	

	#-----------------------------------------------
	# Extract the river network
	# TODO: This can be improved with the DropAnalysis method, but it requires the Outlet of the basin
		
	if test -f ${Outdir}${file_name}sa.tif; then
		echo "${Outdir}${file_name}sa.tif exists"
	else
		mpiexec -np $nproc slopearea ${Outdir}${file_name}.tif
	fi
	if test -f ${Outdir}${file_name}p.tif; then
		echo "${Outdir}${file_name}p.tif exists"
	else
		mpiexec -np $nproc d8flowdir ${Outdir}${file_name}.tif
	fi
	if test -f ${Outdir}${file_name}ad8.tif; then
		echo "${Outdir}${file_name}ad8.tif exists"
	else
		mpiexec -np $nproc aread8 ${Outdir}${file_name}.tif
	fi
	if test -f ${Outdir}${file_name}ssa.tif; then
		echo "${Outdir}${file_name}ssa.tif exists"
	else		
		mpiexec -np $nproc d8flowpathextremeup ${Outdir}${file_name}.tif
	fi
	# This will eventually provided by the hydrofabrics, so I am not worrying about this for now		
	if test -f ${Outdir}${file_name}fake_src.tif; then
		echo "${Outdir}${file_name}fake_src.tif exists"
	else	
		mpiexec -np $nproc threshold -ssa ${Outdir}${file_name}ssa.tif -src ${Outdir}${file_name}fake_src.tif -thresh 3000
		
	fi
	# This will latter be modified when the hydrofabrics include the outlet of HUC06 basins. DropAnalysis will be used to define the best threshold for different areas in USA

	if test -f ${Outdir}${file_name}src.tif; then
		echo "${Outdir}${file_name}src.tif exists"
	else
		mpiexec -np $nproc threshold -ssa ${Outdir}${file_name}ssa.tif -src ${Outdir}${file_name}src.tif -thresh 300
	fi


	if [[ $Variable == *"GIUH"* ]]; then
	
		#Generate travel time in minutes/meter per pixel
		python generate_travel_time_by_pixel.py ${file_name} ${Dir}${out_dir_taudem}/${hucid}/ ${Dir}${hydrofabrics_directory}  --method=${method} --manning=${Dir}${manning}
	
		#Generate travel time in minutes accumulated over the network
		#7/30/2021 - Change from "-m ave h" to "-m ave s" - calculate distance based on the The along the surface difference in elevation between grid cells (s=h*sqrt(1+slope2)
		if test -f ${Outdir}${file_name}dsave${method}_cr.tif; then
			echo "${Outdir}${file_name}dsave${method}_cr.tif exists"
		else	
			echo mpiexec -np $nproc dinfdistdown -ang ${Outdir}${file_name}ang.tif -fel ${FelPath} -src ${Outdir}${file_name}hf.tif -wg ${Outdir}${file_name}wg${method}.tif -dd ${Outdir}${file_name}dsave${method}.tif -m ave s
			mpiexec -np $nproc dinfdistdown -ang ${Outdir}${file_name}ang.tif -fel ${FelPath} -src ${Outdir}${file_name}hf.tif -wg ${Outdir}${file_name}wg${method}.tif -dd ${Outdir}${file_name}dsave${method}.tif -m ave s
			
			#crop the raster to HUC06
			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${Dir}${out_dir_taudem}/${hucid}/${hucid}-wbd.shp -dstalpha ${Outdir}${file_name}dsave${method}.tif -dstnodata "-999.0" ${Outdir}${file_name}dsave${method}_cr.tif
		fi
	
		
		#generate GIUH per basin
		
		python generate_giuh_per_basin_params.py ${hucid} ${Dir}${hydrofabrics_directory}catchments_wgs84.json ${Dir}${out_dir_taudem}/${hucid}/${file_name}dsave${method}_cr.tif ${Dir}$soil_param_file ${Dir}$GW_param_file ${Dir}$out_dir_giuh --output 1 --buffer 0.001 --nodata -999
	fi

	if [[ $Variable == *"TWI"* ]]; then

		if test -f ${Outdir}${file_name}dsave_noweight_cr.tif; then
			echo "${Outdir}${file_name}dsave_noweight.tif exists"
		else
		#7/30/2021 - Calculate the distance downstream - used to generate the width function for topmodel

			mpiexec -np $nproc dinfdistdown -ang ${Outdir}${file_name}ang.tif -fel ${FelPath} -src ${Outdir}${file_name}hf.tif -dd ${Outdir}${file_name}dsave_noweight.tif -m ave s

			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${Dir}${out_dir_taudem}/${hucid}/${hucid}-wbd.shp -dstalpha ${Outdir}${file_name}dsave_noweight.tif -dstnodata "-999.0" ${Outdir}${file_name}dsave_noweight_cr.tif	
		
		fi	

		echo "Generating histogram"						
		#conda activate $python_env
		#generate TWI per basin - need to modify to also generate width function
		

		python generate_twi_per_basin.py ${hucid} ${Dir}${hydrofabrics_directory}catchments_wgs84.json ${Outdir}${file_name}twi_cr.tif ${Outdir}${file_name}slp_cr.tif ${Outdir}${file_name}dsave_noweight.tif ${Dir}$out_dir_twi --output 1 --buffer 0.001 --nodata -999
		
	fi	

	END_TIME=$(date +%s)
	echo "running ${hucid}, for ${Variable}, at ${Resolution} m resolution"
	echo "It took $(($END_TIME-$START_TIME)) seconds to process ${file_name}" 
done 

