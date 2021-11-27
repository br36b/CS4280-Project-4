#!/bin/bash

TEST_FILES="./test_files/P3/*"

for f in $TEST_FILES
do
  echo "$f"
  ./statSem $f

done
