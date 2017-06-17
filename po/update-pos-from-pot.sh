#!/bin/bash
set -e

POT="day.pot"

pos=$(ls | grep \.po$)

for po in $pos; do
    msgmerge -o $po.tmp $po $POT
    mv $po.tmp $po
done

exit 0

