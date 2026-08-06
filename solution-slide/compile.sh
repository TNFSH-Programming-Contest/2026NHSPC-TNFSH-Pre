#!/bin/bash
mkdir -p build
xelatex -output-directory=build -aux-directory=build -shell-escape main.tex
cp build/main.pdf ./target.pdf
