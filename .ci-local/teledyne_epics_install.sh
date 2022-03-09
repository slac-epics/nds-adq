#!/bin/bash 

# copyright (c) Tessella 2016-2017

# This script installs NDS and NDS-epics
#
# check if user has right permissions
if [ "$(id -u)" != "0" ]; then
    echo "Sorry, you are not root. Please try again using sudo."
    exit 1
fi

# terminate script after first line that fails
set -e


DEFAULT_EPICS_ROOT="/usr/local/epics"
if [ "$#" -lt 1 ]; then EPICS_ROOT=$DEFAULT_EPICS_ROOT; else EPICS_ROOT=$1;fi

DEFAULT_EPICS_BASE=$EPICS_ROOT/base
if [ "$#" -lt 2 ]; then EPICS_BASE=$DEFAULT_EPICS_BASE; else EPICS_BASE=$EPICS_ROOT/$2;fi

DEFAULT_SUPPORT_PATH=$EPICS_ROOT/support
if [ "$#" -lt 3 ]; then SUPPORT_PATH=$DEFAULT_SUPPORT_PATH; else SUPPORT_PATH=$EPICS_ROOT/$3;fi

DEFAULT_ASYN_PATH=$SUPPORT_PATH/asyn/current
if [ "$#" -lt 4 ]; then ASYN_PATH=$DEFAULT_ASYN_PATH; else ASYN_PATH=$SUPPORT_PATH/$4;fi

if [ ! -f /etc/redhat-release ]; then    
    # Required build tools
    apt-get -y install cmake
else
    yum install -y cmake
fi

NDS_EPICS_PATH=$SUPPORT_PATH/nds-epics
NDS_EPICS_DOWNLOAD="v3.1.0.tar.gz"

apt install -y ./ADQAPI/libadq0_0.57946_amd64.deb

if [ -d $NDS_EPICS_PATH ];
then
    echo NDS3 is already installed!
    exit 0
fi

mkdir -p $NDS_EPICS_PATH

wget --tries=3 --timeout=10  https://github.com/NDSv3/nds-epics/archive/refs/tags/$NDS_EPICS_DOWNLOAD

tar xzvf $NDS_EPICS_DOWNLOAD -C $SUPPORT_PATH
mv -T $SUPPORT_PATH/nds-epics-3.1.0 $NDS_EPICS_PATH
rm $NDS_EPICS_DOWNLOAD

NDS_DOWNLOAD="v3.2.0.tar.gz"
NDS_PATH=$NDS_EPICS_PATH/nds-core

wget --tries=3 --timeout=10  https://github.com/NDSv3/nds-core/archive/refs/tags/$NDS_DOWNLOAD
tar xzvf $NDS_DOWNLOAD -C $NDS_EPICS_PATH
mv -T $NDS_EPICS_PATH/nds-core-3.2.0 $NDS_PATH
rm $NDS_DOWNLOAD
echo sed "s/return m_driverName;/return m_driverName.c_str();/" $NDS_PATH/include/nds3/registerDevice.h
sed -i "s/return m_driverName;/return m_driverName.c_str();/" $NDS_PATH/include/nds3/registerDevice.h
# Build nds
NDS_BUILD_PATH=$NDS_PATH/build
mkdir $NDS_BUILD_PATH
pushd $NDS_BUILD_PATH
cmake ../CMake
make -j2 install
popd

#build nds examples code - it's required by nds-epics
NDS_EXAMPLES_BUILD_PATH=$NDS_PATH/doc/examples/build
mkdir $NDS_EXAMPLES_BUILD_PATH
pushd $NDS_EXAMPLES_BUILD_PATH
cmake ../CMake -DLIBRARY_LOCATION=../../../build
make -j2 install
popd

#Build nds-epics
# hack the 'RELEASE' file to put the settings we want.

sed -i -e "/^MODULES\s*=/ s,=.*,=$SUPPORT_PATH," $NDS_EPICS_PATH/configure/RELEASE
sed -i -e "/^ASYN\s*=/ s,=.*,=$ASYN_PATH," $NDS_EPICS_PATH/configure/RELEASE
sed -i -e "/^EPICS_BASE\s*=/ s,=.*,=$EPICS_BASE," $NDS_EPICS_PATH/configure/RELEASE

make -j2 -C $NDS_EPICS_PATH install
