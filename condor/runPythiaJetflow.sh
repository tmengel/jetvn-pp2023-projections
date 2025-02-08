#!/bin/bash

### Set up the environment for the job
INSTA=/sphenix/user/tmengel/jetvn-pp2023-projections/install
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source $OPT_SPHENIX/bin/setup_local.sh $INSTA

### Read arguments from the command line
PROCESS=$1
NEVENTS=$2
ROOTDIR=/sphenix/user/tmengel/jetvn-pp2023-projections/condor/rootfiles
if [ ! -d $ROOTDIR ]; then
    mkdir $ROOTDIR
fi

OUTPUTFILE="${ROOTDIR}/jetflow_pythia8_jet10gev-${PROCESS}.root"
outputfile_arg="\"${OUTPUTFILE}\""

#### Echo arguments
echo "outputfile_arg: ${outputfile_arg}"
echo "NEVENTS: ${NEVENTS}"

### Run the macro
cd /sphenix/user/tmengel/jetvn-pp2023-projections/macro
root -b -l -q 'Fun4All_G4_sPHENIX.C('${NEVENTS}','${outputfile_arg}')'

res=$?
echo "res: ${res}"
if [ $res -ne 0 ]; then
    echo "ERROR CODE FOR THIS JOB: ${res}"
    rm -f ${OUTPUTFILE}
    exit
fi

echo "Script done"
# # See if list of output files already exists
OUTPUTFILELOG="/sphenix/user/tmengel/jetvn-pp2023-projections/condor/jetflow_pythia8_jet10gev.log"
echo ${OUTPUTFILE} >> ${OUTPUTFILELOG}








