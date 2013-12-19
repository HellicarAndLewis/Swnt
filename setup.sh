#!/bin/sh

pd=${PWD}

# check dependencies
if [ ! -d extern ] ; then
    mkdir extern
    cd extern
    curl -O http://v3.roxlu.com/downloads/swnt_extern.zip
    unzip swnt_extern.zip
fi

cd extern 
if [ -d __MACOSX ] ; then 
    rm -r __MACOSX
fi


cd ${pd}
if [ ! -d shared ] ; then 
    mkdir shared
    cd shared
    git clone git@github.com:roxlu/tinylib.git 
fi
