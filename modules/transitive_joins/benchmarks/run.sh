if [ $# -ne 3 ]; then
    echo "USAGE: ./run.sh <BENCHMARK> <NUM_WORKERS> <WORKLOAD>"
    echo "WORKLOAD=large, huge"
    echo "BENCHMARK= 1 .... 5"
    echo "1. Tile Smithwaterman with Promise LCA [Workloads are in the 'input' folder]"
    echo "2. Cell Smithwaterman with Promise LCA [Workload is hardcoded to 'inputA' and inputB'"
    echo "3. Cell Smithwaterman with Future LCA [Workload is hardcoded to 'inputA' and inputB'"
    echo "4. Mergesort With Future LCA [large=2000000, huge=200,000,000]"
    echo "5. Mergesort With Promise LCA [large=2000000, huge=200,000,000]"
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

        NUMBERS=2000000
else
if [ "$SIZE" == "huge" ]; then
        TILE_WIDTH=256
        TILE_HEIGHT=256
        INNER_TILE_WIDTH=725
        INNER_TILE_HEIGHT=750
        EXPECTED_RESULT=364792

        NUMBERS=200000000
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
else
if [ "$BENCHMARK" == "3" ]; then
        EXECUTABLE_NAME=CellSmithwatermanFutureLCA

        echo "./${EXECUTABLE_NAME}.out"
        ./${EXECUTABLE_NAME}.out
else
if [ "$BENCHMARK" == "4" ]; then
        EXECUTABLE_NAME=MergesortFutureLCA

        echo "./${EXECUTABLE_NAME}.out ${NUMBERS}"
        ./${EXECUTABLE_NAME}.out ${NUMBERS}
else
if [ "$BENCHMARK" == "5" ]; then
        EXECUTABLE_NAME=MergesortPromiseLCA

        echo "./${EXECUTABLE_NAME}.out ${NUMBERS}"
        ./${EXECUTABLE_NAME}.out ${NUMBERS}
fi
fi
fi
fi
fi


