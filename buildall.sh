#!/bin/bash

set echo on

echo "Building everything ... "
echo "------------------------"

pushd engine
source build.sh
popd

ERRORLEVEL=$?

if [ $ERRORLEVEL -ne 0 ]
then
    echo "Error:"$ERRORLEVEL && exit
fi


pushd testbed
source build.sh
popd

ERRORLEVEL=$?

if [ $ERRORLEVEL -ne 0 ]
then
    echo "Error:"$ERRORLEVEL && exit
fi

echo " Successfully build all libs "
