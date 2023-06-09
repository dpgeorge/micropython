#!/bin/sh
if [ $1 == "-X" ]; then
    shift 2
fi
node $1
