#!/bin/bash

set -x
ulimit -n 8192

benchmark=$1
shift 1

implementation=$1
shift 1

./$implementation &
implementation_pid=$!

sleep 1

# Make sure the implementation is killed if we exit
trap "kill $implementation_pid" EXIT

for c in $*
do
	./$benchmark -c $c
done
