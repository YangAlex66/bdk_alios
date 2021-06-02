#! /bin/bash
# Usage: clean_build.sh

echo "cleaning build ..."

./build.sh clean
if [ $? != 0 ]; then
	echo "clean build error!"
	exit 1
fi
rm -rf out

echo "clean build done."
