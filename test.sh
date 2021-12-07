#!/bin/bash

TEST_FILES="./test_files/P4/*"

for f in $TEST_FILES
do
  echo "$f"
  ./compfs $f

done
