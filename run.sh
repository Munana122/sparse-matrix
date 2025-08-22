#!/bin/bash

# Move to the directory where you can run SparseMAtrix.cpp
cd dsa/code || { echo "Directory not found!"; exit 1; }

# Compile the C++ file (replace 'program.cpp' with your file name)
g++ SparseMatrix.cpp -o sparseM

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running program..."
    ./sparseM
else
    echo "Compilation failed."
fi



