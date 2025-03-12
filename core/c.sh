#!/bin/sh

cmake --build "build"
if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

build/core_test
