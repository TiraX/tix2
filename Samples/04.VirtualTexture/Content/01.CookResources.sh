#!/bin/bash
# Copyright 2018~2020 zhaoshuai. All rights reserved.

if [ ! -d "Cooked" ]; then
	echo "Create directories for cooked resource."
	mkdir Cooked
	pushd Cooked
	mkdir iOS
	popd
fi

echo "Converting tjs files."
Converter="$(dirname "$0")"/../../../Binary/Mac/ResConverter
for fn in $(find . -name '*.tjs'); do
	echo " Converting $fn"
	pathname=$(dirname $fn)
	pathname=${pathname#"./"}
	filename=$(basename $fn .tjs)
	# echo --- $filename
	# echo ---- $pathname
	"$Converter" "$fn" ./Cooked/iOS/"$pathname"/"$filename".tasset -ForceAlphaChannel -astc_quality=high
done

echo "Baking virtual texture pages."
VTBaker="$(dirname "$0")"/../../../Binary/Mac/VTTextureBaker
# "$VTBaker" showcase_01.tjs Cooked/iOS -DumpAllVTs -DumpAllVTWithBorder -DumpAllPages -IgnoreBorders -DebugBorders -VTSize=16384 -PPSize=256 -FillAllPages
"$VTBaker" showcase_04.tjs Cooked/iOS
# Convert scene file again with vt_info
"$Converter" showcase_04.tjs Cooked/iOS/showcase_04.tasset -VTInfo=showcase_04_vt.tjs

echo "Copying Config"
pushd "Cooked/iOS"
if [ ! -d "Config" ]; then
	mkdir Config
fi
popd
cp ./Config/*.ini ./Cooked/iOS/Config/