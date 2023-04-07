cmake_minimum_required(VERSION 3.20)
project("MDO")

set(CMAKE_CXX_STANDART 20)

add_executable(kl knapsack_lists/knapsack_lists.cpp)
add_executable(approx lagrangian_approx/multidim_knapsack.cpp)