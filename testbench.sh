#!/bin/bash

# TODO: Do N test repeats to allow for local scheduling jitter

benchmark=$1
shift 1

implementation=$1
shift 1

# One filehandle per and of the socket and some extra
max_filehandles=$(jq -s 'max * 2 + 10' <<< $*)

echo "Setting ulimit to ${max_filehandles}" >&2
ulimit -n $max_filehandles

./$implementation &
implementation_pid=$!

# Wait for implementation to be listening
until nc -z 127.0.0.1 8080; do sleep 0.2; done

# Make sure the implementation is killed if we exit
trap "kill $implementation_pid" EXIT

for c in $*
do
	./$benchmark -c $c
done
