#!/bin/bash
set -euo pipefail
grader=stub.cpp
code1=sixseven.cpp
code2=eightseven.cpp
problem=Cake_4
g++ -std=gnu++17 -O2 -Wall -Wextra "$grader" "$code1" "$code2" -o "$problem"
