# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pong_main)

# Add include directory

target_include_directories (app PRIVATE
  src/
  modules/arcade-drivers/
)

target_sources(app PRIVATE 
  # src
  src/main.c
  src/pong_oled.c

  # submodule--need these linked with pong and not
  # as separate library (or could we do that)
  modules/arcade-drivers/graphics_core.c
  modules/arcade-drivers/oled_core.c
)
