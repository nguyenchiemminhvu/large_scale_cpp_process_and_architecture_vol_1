cmake_minimum_required(VERSION 3.16)
project(ch1_compilers_linkers_components LANGUAGES CXX)

# Component library: my_stack
# Demonstrates a proper component (.h + .cpp)
add_library(my_stack STATIC
    src/my_stack.cpp
)
target_include_directories(my_stack PUBLIC src)
target_compile_features(my_stack PRIVATE cxx_std_17)

# Component library: linkage_demo
add_library(linkage_demo STATIC
    src/linkage_demo.cpp
)
target_include_directories(linkage_demo PUBLIC src)
target_compile_features(linkage_demo PRIVATE cxx_std_17)

# Component library: include_guard_demo
add_library(include_guard_demo STATIC
    src/include_guard_demo.cpp
)
target_include_directories(include_guard_demo PUBLIC src)
target_compile_features(include_guard_demo PRIVATE cxx_std_17)

# Test driver for my_stack (demonstrates hierarchical testability)
add_executable(ch1_my_stack_test
    src/my_stack.t.cpp
)
target_link_libraries(ch1_my_stack_test PRIVATE my_stack)
target_include_directories(ch1_my_stack_test PRIVATE src)
target_compile_features(ch1_my_stack_test PRIVATE cxx_std_17)

# Full component demo
add_executable(ch1_component_demo
    src/component_demo.cpp
)
target_link_libraries(ch1_component_demo PRIVATE my_stack linkage_demo include_guard_demo)
target_include_directories(ch1_component_demo PRIVATE src)
target_compile_features(ch1_component_demo PRIVATE cxx_std_17)

# Linkage demo
add_executable(ch1_linkage_demo
    src/linkage_demo_main.cpp
)
target_link_libraries(ch1_linkage_demo PRIVATE linkage_demo)
target_include_directories(ch1_linkage_demo PRIVATE src)
target_compile_features(ch1_linkage_demo PRIVATE cxx_std_17)
