#!/bin/bash

export PROGRAM_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd build/
cmake ../
if make ; then
    ./program
else
    echo "Build failed."
fi

