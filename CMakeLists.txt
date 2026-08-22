# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pong_main)

# Library definition for graphics
add_library(graphics_lib STATIC
  modules/arcade-drivers/graphics_core.c
  modules/arcade-drivers/graphics_core.h
  modules/arcade-drivers/oled_core.c
  modules/arcade-drivers/oled_core.h
  modules/arcade-drivers/graphics_common.c
  modules/arcade-drivers/graphics_common.h
)

target_link_libraries(graphics_lib PUBLIC zephyr_interface)

# target_include_directories(graphics_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add include directories
target_include_directories(app PRIVATE
  modules/arcade-drivers/
  inc/
)

target_sources(app PRIVATE 
  # submodule--need these linked with pong and not
  # as separate library (or could we do that)
  # src
  src/main.c
  src/pong_core.c
  src/pong_threads.c

  # modules/arcade-drivers/graphics_core.c
  # modules/arcade-drivers/oled_core.c
  # modules/arcade-drivers/graphics_common.c
)

target_link_libraries(app PRIVATE graphics_lib)
