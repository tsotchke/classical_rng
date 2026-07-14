cmake_minimum_required(VERSION 3.16)
project(classical_rng_consumer LANGUAGES C)

find_package(classical_rng 2 CONFIG REQUIRED)

add_executable(classical_rng_consumer main.c)
target_link_libraries(classical_rng_consumer PRIVATE classical_rng::classical_rng)
