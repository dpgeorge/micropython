#!/bin/sh

run_tests=../../../tests/run-tests.py

$run_tests -r ./results --clean-failures
MICROPY_MICROPYTHON=$(pwd)/node_run.sh $run_tests --skip-feature-detection -r ./results basics/*.js basics/*.mjs
$run_tests -r ./results --print-failures
