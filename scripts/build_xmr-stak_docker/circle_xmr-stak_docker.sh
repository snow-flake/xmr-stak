#!/bin/bash -uex

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

########################
# Ubuntu 16.04
########################

set -x ;
apt update -qq ;
apt install -y -qq cmake g++ libmicrohttpd-dev libssl-dev libhwloc-dev wget;

wget -c https://developer.nvidia.com/compute/cuda/9.0/Prod/local_installers/cuda_9.0.176_384.81_linux-run;
chmod a+x cuda_*_linux-run;
./cuda_*_linux-run --silent --toolkit ;

cmake -DCUDA_ENABLE=ON -DOpenCL_ENABLE=OFF .;
make;
