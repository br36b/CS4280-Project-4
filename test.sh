#!/bin/bash

TEST_FILES="./test_files/P4/*.fl2021"

for f in $TEST_FILES
do
  echo "$f"
  ./compfs $f

done
