cmake_minimum_required(VERSION 3.16)
project(ch0_motivation LANGUAGES CXX)

# --- Demo 1: Software Capital ---
add_executable(ch0_software_capital
    src/software_capital_demo.cpp
)
target_include_directories(ch0_software_capital PRIVATE src)
target_compile_features(ch0_software_capital PRIVATE cxx_std_17)

# --- Demo 2: Hierarchical Reuse ---
add_executable(ch0_hierarchical_reuse
    src/hierarchical_reuse_demo.cpp
)
target_include_directories(ch0_hierarchical_reuse PRIVATE src)
target_compile_features(ch0_hierarchical_reuse PRIVATE cxx_std_17)
