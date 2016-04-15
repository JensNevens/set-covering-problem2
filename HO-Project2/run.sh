#!/bin/sh

#  run.sh
#  HO-Project2
#
#  Created by Jens Nevens on 15/04/16.
#  Copyright © 2016 Jens Nevens. All rights reserved.

# $1 is path of scp executable
# $2 is path of folder with instances
# $3 is path of folder to save results

seed=333

# Remove old files
for f in "$3"/*.txt
do
    rm ${f}
done

# Run ACO
for instance in "$2"/*
do
    y=${instance%.txt} # get instance name
    cost=$(eval "$1 --seed $seed --instance $instance") #This can use additional parameters
    filename=${y##*/}
    length=${#filename}
    first=$(echo ${filename:3:$length-4} | awk '{print toupper($0)}')
    second=${filename:$length-1:1}
    echo "$first.$second $cost" >> "$3/aco.txt"
done
