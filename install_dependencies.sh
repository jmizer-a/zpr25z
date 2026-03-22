#!/bin/bash

sudo apt update
sudo apt install build-essential cmake qt6-base-dev libarmadillo-dev libmlpack-dev libensmallen-dev clang-tidy clang-format pkg-config
echo -e "\033[1;32m=== Installation Complete ===\033[0m"
