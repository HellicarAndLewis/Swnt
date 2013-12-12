#!/bin/bash
set -x
# addons directory
ad=${PWD}/Addons/  
ofad=${PWD}/../../addons/

if [ ! -d ${ad} ] ; then 
    mkdir ${ad}
fi

# 1 = git url
# 2 = sub directory
# 3 = revision to use
# 4 = branch to use

function clone {
    url=${1}
    dir=${2}
    rev=${3}
    branch=${4}
    if [ ! -d ${ad}/${dir} ] ; then 
        mkdir ${ad}/${dir}
        cd ${ad}/${dir}
        git clone ${url} .
    fi

    if [ -d ${ofad}/${dir} ] ; then 
        echo "********************************************************************************************************************************************************************"
        echo "It looks like you cloned the ${url} already in the addons directory of OF. This might cause compile errors because XCode gets confused about what headers to include!"
        echo "********************************************************************************************************************************************************************"
    fi

    cd ${ad}/${dir}
    git checkout ${branch}
    git reset --hard ${rev}
}


clone git@github.com:vanderlin/ofxBox2d.git ofxBox2d 770257186b28833a4e78b5ecd68f05a8bf18053c master
