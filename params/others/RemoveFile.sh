
declare -a HUC=("010100" "010200" "010300" "010400" "010500" "010600" "010700" "010801" "010900" "011000" "041505" "010802") # HUCS to process - this was process for the demo
dem_dir=~/Projects/IUH_TWI/HAND_DEM/ 	
out_dir_taudem=~/Projects/IUH_TWI/HAND_30m/ 
for val in  ${HUC[@]}; do
	hucid=$val
	cd ${out_dir_taudem}
	mkdir $hucid
	cd $hucid
	file_name=${hucid}_30m	
	rm ${file_name}dsave_noweight_cr.tif
	rm ${file_name}dsave1_cr.tif
	
done 
