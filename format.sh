#!/bin/bash

find -name '*.hpp' -print | xargs -n 1 clang-format -i style=file >/dev/null 2>&1
find -name '*.tpp' -print | xargs -n 1 clang-format -i style=file >/dev/null 2>&1
find -name '*.ipp' -print | xargs -n 1 clang-format -i style=file >/dev/null 2>&1
find -name '*.cpp' -print | xargs -n 1 clang-format -i style=file >/dev/null 2>&1
