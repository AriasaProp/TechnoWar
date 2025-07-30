#!/bin/bash

set -e

# validate argument
if [ -z "$1" ]; then
  echo "Usage: $0 <file_input>"
  echo "Example: $0 my_data.txt"
  exit 1
fi

INPUT_FILE="$1"

# check if file is exist or readable
if [ ! -f "$INPUT_FILE" ]; then
  echo "Error: File '$INPUT_FILE' not found."
  exit 1
fi
if [ ! -r "$INPUT_FILE" ]; then
  echo "Error: File '$INPUT_FILE' not readable."
  exit 1
fi

echo "--- Processing '$INPUT_FILE' ---"

# finding pattern

sed -n '/#ifndef VK_NO_PROTOTYPES/,/#endif/p' "$INPUT_FILE"


echo "--- Done! '$INPUT_FILE' ---"
