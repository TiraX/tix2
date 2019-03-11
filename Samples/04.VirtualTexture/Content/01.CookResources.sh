#!/bin/bash
# Copyright 2018 zhaoshuai. All rights reserved.

if [ ! -d "Cooked" ]; then
	echo "Create directories for cooked resource."
	mkdir Cooked
	pushd Cooked
	mkdir iOS
	popd
fi

echo "Converting tjs files."
Converter="$(dirname "$0")"/../../../Binary/Mac/ResConverter
for fn in *.tjs; do
	echo "Converting $fn"
	"$Converter" "$fn" ./Cooked/iOS/"${fn%.*}".tres
done

echo "Copying Config"
pushd "Cooked/iOS"
if [ ! -d "Config" ]; then
	mkdir Config
fi
popd
cp ./Config/*.ini ./Cooked/iOS/Config/

echo "Copying other data files"
for fn in *.bn; do
	cp "$fn" ./Cooked/iOS/"$fn"
done