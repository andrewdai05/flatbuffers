#!/bin/bash

function build_grpc () {
  git clone https://github.com/grpc/grpc.git google/grpc
  cd google/grpc
  git submodule update --init
  make
  make install prefix=`pwd`/install
  if [ ! -f ${GRPC_INSTALL_PATH}/lib/libgrpc++_unsecure.so.1 ]; then
    ln -s ${GRPC_INSTALL_PATH}/lib/libgrpc++_unsecure.so.6 ${GRPC_INSTALL_PATH}/lib/libgrpc++_unsecure.so.1
  fi
  cd ../..
}

function travis_build () {
  if [ ! -d "travis_build" ]; then
   mkdir travis_build
  fi
  cd travis_build
  cmake -DFLATBUFFERS_BUILD_GRPCTEST=ON -DGRPC_INSTALL_PATH=${GRPC_INSTALL_PATH} -DPROTOBUF_DOWNLOAD_PATH=${PROTOBUF_DOWNLOAD_PATH} ..
  make
  if [ "$OSTYPE" = "linux-gnu" ]
  then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${GRPC_INSTALL_PATH}/lib
    make test ARGS=-V
    # ./grpctest
  elif [ "$OSTYPE" = "darwin17" ]
  then
    export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:${GRPC_INSTALL_PATH}/lib 
    make test ARGS=-V
    # ./grpctest
  fi
  cd ..
}

GRPC_INSTALL_PATH=`pwd`/google/grpc/install
PROTOBUF_DOWNLOAD_PATH=`pwd`/google/grpc/third_party/protobuf

build_grpc
#travis_build
