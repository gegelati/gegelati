#!/bin/sh

DIR=$1
DAT=$2
target=$3

build=${DIR}/buildCodeGen/${target}

mkdir -p ${build}

cd ${DIR}/buildCodeGen/${target}

cmake -DDIR=${DIR} ${DAT}/codeGen/${target}

cd -

cmake --build ${build} --target ${target}