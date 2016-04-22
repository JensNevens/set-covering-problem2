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

# Write header to output file
echo "instance, cost, time" >> "$3/ACO/run.txt"

# Run ACO
for instance in "$2"/*
do
    y=${instance%.txt}
    output=$(eval "$1 --seed $seed --instance $instance --runtime 10 --aco --rep")

    filename=${y##*/}
    length=${#filename}
    first=$(echo ${filename:3:$length-4} | awk '{print toupper($0)}')
    second=${filename:$length-1:1}

    echo "$first.$second, $output" >> "$3/ACO/run.txt"
done

# Write header to output file
echo "instance, cost, time" >> "$3/GEN/run.txt"

# Run ACO
for instance in "$2"/*
do
    y=${instance%.txt}
    output=$(eval "$1 --seed $seed --instance $instance --runtime 10 --ga --pops 100 --prop --fusion --bi")

    filename=${y##*/}
    length=${#filename}
    first=$(echo ${filename:3:$length-4} | awk '{print toupper($0)}')
    second=${filename:$length-1:1}

    echo "$first.$second, $output" >> "$3/GEN/run.txt"
done
