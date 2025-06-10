#!/bin/bash

find -name '*.hpp' -o -name '*.tpp' -o -name '*.cpp' -print | xargs -n 1 clang-format -i style=file >/dev/null 2>&1
