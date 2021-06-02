#! /bin/bash
# Usage: make_build.sh [platform]

source ./tools/scripts/build_include.sh

target_type="example"
target_app="beken_test"
target_board="bk7231ndevkitc"

if [ "$1" != "" ]; then
	PLATFORM=$1
else
	PLATFORM=bk7231n
fi

validate_platform $PLATFORM
if [ $? != 0 ]; then
	exit 1
fi

if [ "$PLATFORM" == "bk7231u" ]; then
	target_board="bk7231udevkitc"
fi

PREV_BOARD_FILE=.board
if [ -f $PREV_BOARD_FILE ]; then
	PREV_BOARD=$(cat $PREV_BOARD_FILE)
	if [ "$PREV_BOARD" != "$target_board" ]; then
		echo "cleaning $PREV_BOARD ..."
		./tools/scripts/clean_build.sh
	fi
	rm -f $PREV_BOARD
fi

echo "making build for $PLATFORM ..."

./build.sh ${target_type} ${target_app} ${target_board}
if [ $? != 0 ]; then
	echo "make build error!"
	exit 1
else
	echo "make build done."
fi
