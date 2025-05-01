#!/bin/bash

#!/bin/bash

# Get the number of available cores
num_cores=$(nproc)

# Calculate two-thirds of the number of cores
num_threads=$(($num_cores * 2 / 3))


if [ ! -d "build" ]; then
mkdir build
fi

cd build && cmake .. && make -j$num_threads
