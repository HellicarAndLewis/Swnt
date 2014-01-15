#!/bin/sh

git remote update
git merge origin/master

cd shared/tinylib/
git remote update
git merge origin master
