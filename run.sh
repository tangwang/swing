#!/bin/bash
source  ~/.bash_profile

DAY=`date -d "1 days ago" +"%Y%m%d"`

output_dir=output_${DAY}
mkdir ${output_dir}

cat ./data/${DAY}/*  | bin/swing bin/swing 0.7 0.001 0.8 23 ${output_dir} 0  
cat ./data/${DAY}/*  | bin/swing_symmetric 0.8 1.0 0
cat ./data/${DAY}/*  | bin/swing_1st_order 0.1 0.5 1 1 

