#!/bin/sh

clang++ -Ofast ./ar-wrapper.cpp -o ./ar-wrapper
clang++ -Ofast ./lld-wrapper.cpp -o ./lld-wrapper
