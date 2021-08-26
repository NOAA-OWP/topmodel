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


mkdir ${dem_dir}
mkdir ${out_dir_taudem}
mkdir ${out_dir_twi}
mkdir ${out_dir_giuh}

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

	cd ${dem_dir}	
	#echo "Check for file ${hucid}fel.tif\n"
	if test -f ${hucid}fel.tif; then
		echo "${hucid}fel.tif exists"
	else
		if test -f ${out_dir_taudem}/${hucid}/${file_name}fel.tif; then
			echo  ${out_dir_taudem}/${hucid}/${file_name}fel.tif
		else
	 		echo "Downloading file ${hucid}fel.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}fel.tif -o ${hucid}fel.tif
 		fi
	fi
#	END_TIME=$(date +%s)  
	
	cd ${out_dir_taudem}
	mkdir $hucid
	cd $hucid
	#-----------------------------------------------
	# Download shapefile with area
	if test -f ${hucid}-wbd.shp; then
		echo "${hucid}-wbd.shp exists"
	else
        	echo "Downloading file ${hucid}-wbd.shp"
	  	curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.dbf -o ${hucid}-wbd.dbf
		curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.prj -o ${hucid}-wbd.prj
	  	curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.shx -o ${hucid}-wbd.shx
		curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}-wbd.shp -o ${hucid}-wbd.shp	
	fi
	#END_TIME6=$(date +%s)
	#echo "It took $(($END_TIME6-$END_TIME5)) seconds to dowload the data/n"  
		
	if [[ "$Resolution" -eq 30 ]];then
		
		#If resolution equal to 30, aggregate DEM and run pitremove and dinfflowdir
		#-----------------------------------------------
		# Resample DEM to 30 x 30 meters
		if test -f ${file_name}.tif; then
			echo "${file_name}.tif exists"
		else
			echo "Resampled DEM to 30 x 30 meters"
			gdalwarp -tr 0.0003086429087 0.0003086429087 -r average ${dem_dir}/${hucid}fel.tif ${file_name}.tif
		fi
		FelPath=${out_dir_taudem}/${hucid}/${file_name}fel.tif

		#-----------------------------------------------
		# pitremove  
		echo "Process DEM"
		if test -f ${file_name}fel.tif; then
			echo "${file_name}fel.tif exists"
		else
			mpiexec -np $nproc pitremove -z ${file_name}.tif -fel ${FelPath}
		fi 	
		
		#-----------------------------------------------
		# dinfflowdir  
		if test -f ${file_name}ang.tif; then
			echo "${file_name}p.tif exists\n"
		else
			mpiexec -np $nproc dinfflowdir -ang ${file_name}ang.tif -slp ${file_name}slp.tif -fel ${FelPath}
		fi
					
	else
	
		#If resolution equal to 10, download the info from the website 
		FelPath=${dem_dir}/${file_name}fel.tif
		#-----------------------------------------------
		# Download other available datasets
		if test -f ${file_name}slp.tif; then
			echo "${file_name}slp.tif exists"
		else
		 	echo "Downloading file ${hucid}slp.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}slp.tif -o ${file_name}slp.tif
		fi
				
		if test -f ${file_name}ang.tif; then
			echo "${file_name}ang.tif exists"
		else
		 	echo "Downloading file ${file_name}ang.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}ang.tif -o ${file_name}ang.tif
 	
		fi			
		if test -f ${file_name}p.tif; then
			echo "${file_name}p.tif exists"
		else
		 	echo "Downloading file ${file_name}p.tif"
			curl http://web.corral.tacc.utexas.edu/nfiedata/HAND/${hucid}/${hucid}p.tif -o ${file_name}p.tif
 	
		fi	
	fi

	#-----------------------------------------------
	# areadinf - sca is not available for the 10 meters, so we need to calculate it independently of the resolution
	if test -f ${file_name}sca.tif; then
		echo "${file_name}sca.tif exists"
	else
		mpiexec -np $nproc  areadinf -ang ./${file_name}ang.tif -sca ./${file_name}sca.tif 
	fi
	
	if [[ $Variable == *"TWI"* ]]; then

		#-----------------------------------------------
		# twi
		echo "Generate Topographic Wetness Index"
		if test -f ${file_name}twi.tif; then
			echo "${file_name}twi.tif exists"
		else
		mpiexec -np $nproc twi -slp ${file_name}slp.tif -sca ${file_name}sca.tif -twi ${file_name}twi.tif
		fi

		#-----------------------------------------------
		# Crop TWI and slope
		echo "Crop DEM to the area of interest based on Shapefile"
		if test -f ${file_name}twi_cr.tif; then
			echo "${file_name}twi_cr.tif exists"
		else
			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${hucid}-wbd.shp -dstalpha ${file_name}twi.tif -dstnodata "-999.0" ${file_name}twi_cr.tif
			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${hucid}-wbd.shp -dstalpha ${file_name}slp.tif -dstnodata "-999.0" ${file_name}slp_cr.tif
		fi


		if test -f ${file_name}hf.tif; then
			echo "catchments_wgs84.geojson exists"
		else
		 	echo "Reproject hydrofabrics file catchments_wgs84.geojson"
			ogr2ogr -f "GeoJSON" ${hydrofabrics_directory}catchments_wgs84.geojson ${hydrofabrics_directory}catchments.geojson  -s_srs EPSG:5070 -t_srs EPSG:4326
			ogr2ogr -f "GeoJSON" ${hydrofabrics_directory}flowpaths_wgs84.geojson ${hydrofabrics_directory}flowpaths.geojson -s_srs EPSG:5070 -t_srs EPSG:4326
			gdal_translate -scale 0 40000000000000 0 0 ${file_name}fel.tif ${file_name}hf.tif
			gdal_rasterize -b 1 -burn 1  ${hydrofabrics_directory}flowpaths_wgs84.geojson ${file_name}hf.tif			
		fi

	fi
	

	#-----------------------------------------------
	# Extract the river network
	# TODO: This can be improved with the DropAnalysis method, but it requires the Outlet of the basin
		
	if test -f ${file_name}sa.tif; then
		echo "${file_name}sa.tif exists"
	else
		mpiexec -np $nproc slopearea ${file_name}.tif
	fi
	if test -f ${file_name}p.tif; then
		echo "${file_name}p.tif exists"
	else
		mpiexec -np $nproc d8flowdir ${file_name}.tif
	fi
	if test -f ${file_name}ad8.tif; then
		echo "${file_name}ad8.tif exists"
	else
		mpiexec -np $nproc aread8 ${file_name}.tif
	fi
	if test -f ${file_name}ssa.tif; then
		echo "${file_name}ssa.tif exists"
	else		
		mpiexec -np $nproc d8flowpathextremeup ${file_name}.tif
	fi
	# This will eventually provided by the hydrofabrics, so I am not worrying about this for now		
	if test -f ${file_name}fake_src.tif; then
		echo "${file_name}fake_src.tif exists"
	else	
		mpiexec -np $nproc threshold -ssa ${file_name}ssa.tif -src ${file_name}fake_src.tif -thresh 3000
		
	fi
	# This will latter be modified when the hydrofabrics include the outlet of HUC06 basins. DropAnalysis will be used to define the best threshold for different areas in USA

	if test -f ${file_name}src.tif; then
		echo "${file_name}src.tif exists"
	else
		mpiexec -np $nproc threshold -ssa ${file_name}ssa.tif -src ${file_name}src.tif -thresh 300
	fi


	if [[ $Variable == *"GIUH"* ]]; then
	
		#Generate travel time in minutes/meter per pixel	
		python ~/git_repositories/twi/workflow/generate_travel_time_by_pixel.py ${file_name} ${out_dir_taudem}/${hucid}/ ${hydrofabrics_directory}  --method=${method} --manning=${manning}
	
		#Generate travel time in minutes accumulated over the network
		#7/30/2021 - Change from "-m ave h" to "-m ave s" - calculate distance based on the The along the surface difference in elevation between grid cells (s=h*sqrt(1+slope2)
		if test -f ${file_name}dsave${method}_cr.tif; then
			echo "${file_name}dsave${method}.tif exists"
		else	
			echo mpiexec -np $nproc dinfdistdown -ang ${file_name}ang.tif -fel ${FelPath} -src ${file_name}hf.tif -wg ${file_name}wg${method}.tif -dd ${file_name}dsave${method}.tif -m ave s
			mpiexec -np $nproc dinfdistdown -ang ${file_name}ang.tif -fel ${FelPath} -src ${file_name}hf.tif -wg ${file_name}wg${method}.tif -dd ${file_name}dsave${method}.tif -m ave s
			
			#crop the raster to HUC06
			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${hucid}-wbd.shp -dstalpha ${file_name}dsave${method}.tif -dstnodata "-999.0" ${file_name}dsave${method}_cr.tif
		fi
	
		
		#generate GIUH per basin
		python ~/git_repositories/twi/workflow/generate_giuh_per_basin.py ${hucid} ${hydrofabrics_directory}catchments_wgs84.geojson ${out_dir_taudem}/${hucid}/${file_name}dsave${method}_cr.tif $out_dir_giuh --output 1 --buffer 0.001 --nodata -999


	fi

	if [[ $Variable == *"TWI"* ]]; then

		if test -f ${file_name}dsave_noweight_cr.tif; then
			echo "${file_name}dsave_noweight.tif exists"
		else
		#7/30/2021 - Calculate the distance downstream - used to generate the width function for topmodel
			echo mpiexec -np $nproc dinfdistdown -ang ${file_name}ang.tif -fel ${FelPath} -src ${file_name}hf.ti -dd ${file_name}dsave_noweight.tif -m ave s
			mpiexec -np $nproc dinfdistdown -ang ${file_name}ang.tif -fel ${FelPath} -src ${file_name}hf.tif -dd ${file_name}dsave_noweight.tif -m ave s

			gdalwarp -cutline --config GDALWARP_IGNORE_BAD_CUTLINE YES ${hucid}-wbd.shp -dstalpha ${file_name}dsave_noweight.tif -dstnodata "-999.0" ${file_name}dsave_noweight_cr.tif	
			echo mpiexec -np $nproc dinfdistdown -ang ${file_name}ang.tif -fel ${FelPath} -src ${file_name}hf.tif -wg ${file_name}wg${method}.tif -dd ${file_name}dsave${method}.tif -m ave s
		fi	

		echo "Generating histogram"						
		#conda activate $python_env
		#generate TWI per basin - need to modify to also generate width function
		python ~/git_repositories/twi/workflow/generate_twi_per_basin.py ${hucid} ${hydrofabrics_directory}catchments_wgs84.geojson ${file_name}twi_cr.tif ${file_name}slp_cr.tif ${file_name}dsave_noweight.tif $out_dir_twi --output 1 --buffer 0.001 --nodata -999
		
	fi	

	END_TIME=$(date +%s)
	echo "running ${hucid}, for ${Variable}, at ${Resolution} m resolution"
	echo "It took $(($END_TIME-$START_TIME)) seconds to process ${file_name}" 
done 

