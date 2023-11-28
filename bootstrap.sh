#!/bin/bash

# Stop on first error
set -e

# Check if the vcpkg directory exists
if [ ! -d "vcpkg" ]; then
    echo "Downloading vcpkg..."
    # Clone the vcpkg repository
    git clone https://github.com/microsoft/vcpkg.git

    echo "Building vcpkg..."
    cd vcpkg && ./bootstrap-vcpkg.sh

    cd ..
else
    echo "vcpkg directory already exists, skipping download and build."
fi


# Configure and build the project
echo "Configuring and building the project..."

if [ -d "build" ]; then
    rm -r build
fi

cmake --preset=default

