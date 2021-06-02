#! /bin/bash
# generate SDK for all supported platforms after a new git clone

source ./tools/scripts/build_include.sh

PLATFORMS="$SUPPORTED_BUILD_PLATFORMS"
CLEAN_GIT=0

if [ "$1" != "" ]; then
	if [ "$1" == "-c" ]; then
		CLEAN_GIT=1
	else
		PLATFORMS="$1"
	fi
	if [ "$2" != "" ]; then
		if [ "$2" == "-c" ]; then
			CLEAN_GIT=1
		else
			echo "The second param must be '-c'"
			exit 1
		fi
	fi
fi

for PLATFORM in ${PLATFORMS}
do
	validate_platform $PLATFORM
	if [ $? != 0 ]; then
		exit 1
	fi
done

for PLATFORM in ${PLATFORMS}
do
	./tools/scripts/gen_sdk.sh $PLATFORM
	if [ $? != 0 ]; then
		echo "make $PLATFORM sdk fail"
		exit 1
	fi
done

# remove output and unused files
rm -f .board
rm -rf ./bugzilla
rm -rf Living_SDK/build/compiler

# remove git files
if [ $CLEAN_GIT -eq 1 ]; then
	find ./ -name *.git* | xargs rm -rf
fi

echo "make SDK done."
