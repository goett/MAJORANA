#!/bin/bash
# Convert ORCA output to rootfile record
#usage:
# rootify.sh <inputfilepath> <outputdirectory>
# can be automatically run in a directory with orca2rootdir.py
# J. Goett (goett@lanl.gov) 5/11/12

#log hostname
echo Running on `hostname`
# cd into working directory
echo Output is in $2
cd $2

# -- SGE commands -- #

# set the shell to use
#$ -S /bin/bash
# import all environment variables
#$ -V
# set the current directory as the working directory
#$ -cwd
# join the stdout and stderr into a single log file
#$ -j yes

# -- define tasks -- #

export inpath=$1
export file=$(echo $inpath | sed 's!.*/!!')

# copy the input file over to scratch disk first 

echo 'Processing ' $file 'sourced from ' ${inpath} 
majorcaroot --sis --validatexml --eventwindow 1us --limbufevents 2000 --label ${file}_10us ${inpath} &> $2/log/${file}
