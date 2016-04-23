#!/bin/sh

#  run.sh
#  HO-Project2
#
#  Created by Jens Nevens on 15/04/16.
#  Copyright © 2016 Jens Nevens. All rights reserved.

# $1 is path of scp executable
# $2 is path of folder with instances
# $3 is path of folder to save results
# $4 is path of file with runtimes

seed=333

# Write header to output file
echo "instance, cost, time" >> "$3/runACO.txt"

# Run ACO
line=1
for instance in "$2"/*
do
    rt=$(sed -n "${line}p" "$4")
    line=$(( line + 1 ))

    y=${instance%.txt}
    output=$("$1" --seed $seed --instance $instance --aco --rep --runtime $rt)

    filename=${y##*/}
    length=${#filename}
    first=$(echo ${filename:3:$length-4} | awk '{print toupper($0)}')
    second=${filename:$length-1:1}

    echo "$first.$second, $output" >> "$3/runACO.txt"
done

# Write header to output file
echo "instance, cost, time" >> "$3/runGEN.txt"

# Run ACO
line=1
for instance in "$2"/*
do
    rt=$(sed -n "${line}p" "$4")
    line=$(( line + 1 ))

    y=${instance%.txt}
    output=$("$1" --seed $seed --instance $instance --ga --pops 100 --prop --fusion --bi --runtime $rt)

    filename=${y##*/}
    length=${#filename}
    first=$(echo ${filename:3:$length-4} | awk '{print toupper($0)}')
    second=${filename:$length-1:1}

    echo "$first.$second, $output" >> "$3/runGEN.txt"
done