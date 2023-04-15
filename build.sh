#!/bin/bash


BUILD_TARGET=client
BUILD_TYPE=Release

while getopts "sd" arg
do
  case $arg in
    s)
      echo "building server";
      BUILD_TARGET="server";
      ;;
    d)
      BUILD_TYPE=Debug;
      ;;
    ?)
      echo "unkonw argument"
  exit 1
  ;;
  esac
done

if [[ -d build ]]; then
  echo "Build directory exists";
else
  echo "Create build directory";
  mkdir build
fi

CMAKE_CMD="cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ../"
echo ${CMAKE_CMD}
cd ./build
${CMAKE_CMD}

make -j32

echo "-------------------- build finish ----------------------"
