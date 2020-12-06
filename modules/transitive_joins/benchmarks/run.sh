if [ $# -ne 3 ]; then
	echo "USAGE: ./run.sh <BENCHMARK> <NUM_WORKERS> <WORKLOAD>"
	echo "BENCHMARK= 1"
    echo "1. Tile Smithwaterman with Promise LCA"
    echo "WORKLOAD=large, huge"
	exit
fi

BENCHMARK=$1
export HCLIB_WORKERS=$2
SIZE=$3
export HCLIB_STATS=1

INPUT_FILE_1="./input/string1-$SIZE.txt"
INPUT_FILE_2="./input/string2-$SIZE.txt"

if [ "$SIZE" == "large" ]; then
        TILE_WIDTH=2320
        TILE_HEIGHT=2400
        INNER_TILE_WIDTH=232
        INNER_TILE_HEIGHT=240
        EXPECTED_RESULT=36472
else
if [ "$SIZE" == "huge" ]; then
        TILE_WIDTH=256
        TILE_HEIGHT=256
        INNER_TILE_WIDTH=725
        INNER_TILE_HEIGHT=750
        EXPECTED_RESULT=364792
fi
fi

if [ "$BENCHMARK" == "1" ]; then
        EXECUTABLE_NAME=TileSmithwatermanPromiseLCA

        echo "./${EXECUTABLE_NAME}.out ${INPUT_FILE_1} ${INPUT_FILE_2} ${TILE_WIDTH} ${TILE_HEIGHT} ${INNER_TILE_WIDTH} ${INNER_TILE_HEIGHT}"
        ./${EXECUTABLE_NAME}.out ${INPUT_FILE_1} ${INPUT_FILE_2} ${TILE_WIDTH} ${TILE_HEIGHT} ${INNER_TILE_WIDTH} ${INNER_TILE_HEIGHT}
else
if [ "$BENCHMARK" == "2" ]; then
        EXECUTABLE_NAME=CellSmithwatermanPromiseLCA

        echo "./${EXECUTABLE_NAME}.out"
        ./${EXECUTABLE_NAME}.out
fi
fi


