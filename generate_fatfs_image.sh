#!/bin/bash -e
# Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

usage() {
cat << ==usage
Usage: $(basename $0) [image_dir] [assets_root_dir]
	image_dir Target disk image
	asset_root_dir Asset root directory
==usage
}

ASSETS_DIR="assets country-codes.db Luts.bin"

TEST_ITEMS="testfiles"

if [ $# -ne 2 ]; then
	echo "Error! Invalid argument count"
	usage
	exit -1
fi
IMAGE_NAME=$(realpath $1)
SRC_DATA=$(realpath $2)

if [ ! -d "$SRC_DATA" ]; then
	echo "Error! asset_root_dir is not a directory"
	usage
	exit -1
fi
_REQ_CMDS="sfdisk mtools awk truncate"
for cmd in $_REQ_CMDS; do
	if [ ! $(command -v $cmd) ]; then
		echo "Error! $cmd is not installed, please use 'sudo apt install' for install required tool"
		exit -1
	fi
done
#mtools version
_AWK_SCRIPT='
/[0-9]/ { 
	split($4,vers,"."); 
	if(vers[1]>=4 && vers[2]>=0 && vers[3] >= 24) { 
		print "true"; 
	}
	exit 0; 
}'
MTOOLS_OK=$(mtools --version | awk "${_AWK_SCRIPT}")

if [ ! $MTOOLS_OK ]; then
	echo "Invalid mtools version, please upgrade mtools to >= 4.0.24"
	exit -1
fi

SDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GENLFS=$(find $SDIR -type f -iname genlittlefs -executable -print -quit)
if [ -z ${GENLFS} ]; then
    echo "Unable to find genlilttlefs..."
    exit -1
fi

# Partition sizes in sector 512 units
PART1_START=2048
PART1_SIZE=2097152
PART2_START=$(($PART1_START + $PART1_SIZE))
PART2_SIZE=$PART1_SIZE
PART3_START=$(($PART2_START + $PART2_SIZE))
PART3_SIZE=29358080

truncate -s 16G $IMAGE_NAME
sfdisk $IMAGE_NAME << ==sfdisk
label: dos
label-id: 0x09650eb4
unit: sectors

/dev/sdx1 : start=    $PART1_START,  size=    $PART1_SIZE, type=b, bootable
/dev/sdx2 : start=    $PART2_START,  size=    $PART2_SIZE, type=b
/dev/sdx3 : start=    $PART3_START,  size=    $PART3_SIZE, type=9e
==sfdisk

PART1="$IMAGE_NAME@@$(($PART1_START*512))"
PART2="$IMAGE_NAME@@$(($PART2_START*512))"
mformat -i "$PART1" -F -T $PART1_SIZE -M 512 -v MUDITAOS
mformat -i "$PART2" -F -T $PART2_SIZE -M 512 -v RECOVER
mmd -i "$PART1" ::/current
cd "$SRC_DATA"
for i in $ASSETS_DIR; do
	mcopy -s -i "$PART1" $i ::/current/
done
mcopy -s -i "$PART1" .boot.json ::
mcopy -s -i "$PART1" .boot.json.crc32 ::
mmd -i "$PART1" ::/current/sys
mcopy -s -i "$PART1" data ::/current/sys

# Testing parts of files
for i in $TEST_ITEMS; do
	mcopy -s -i "$PART1" $i ::/current
done
mcopy -s -i "$PART1" sys/updates ::

#Littlefs generate image
$GENLFS --image=$IMAGE_NAME --block_size=4096  --overwrite  --partition_num=3 -- user/*
# back to previous dir
cd -
