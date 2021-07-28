#!/bin/sh

DIR=$PWD
DEBUG=$1
DAT=$2
target=$3

build=${DIR}/buildCodeGen/${target}

mkdir -p ${build}

cd ${build}

cmake -DDIR=${DIR} -DDEBUG=${DEBUG} ${DAT}/codeGen/${target}

cd -

cmake --build ${build} --target ${target}