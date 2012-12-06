#!/bin/sh
find ../bin -name ".clang_complete" -exec cat {} >> .clang_complete_temp \;

# Remove duplicates
sort .clang_complete_temp | uniq > .clang_complete
mv .clang_complete ../
rm .clang_complete_temp

