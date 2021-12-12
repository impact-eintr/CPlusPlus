#!/usr/bin/env bash

make

# 需要两个终端
# 1 server
./server

# 2 client
./client 8 |grep qps
