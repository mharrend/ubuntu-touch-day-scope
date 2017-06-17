#!/bin/bash

pos=$(ls | grep po$)
for po in $pos
do
    echo $po
    [ -a ../../unity-scope-dashboard/po/$po ] && msgmerge -o ${po}.new ../../unity-scope-dashboard/po/$po $po
    mv $po.new $po
done
