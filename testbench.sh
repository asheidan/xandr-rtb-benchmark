#!/bin/bash

benchmark=$1
shift 1

implementation=$1
shift 1

max_filehandles=$(jq -s 'max * 2' <<< $*)

echo "Setting ulimit to ${max_filehandles}" >&2
ulimit -n $max_filehandles
./$implementation &
implementation_pid=$!

sleep 1

# Make sure the implementation is killed if we exit
trap "kill $implementation_pid" EXIT

for c in $*
do
	./$benchmark -c $c
done
