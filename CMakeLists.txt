cmake_minimum_required(VERSION 3.16)
project(LargeScaleCpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Sub-projects (one per chapter)
add_subdirectory(ch0_motivation)
add_subdirectory(ch1_compilers_linkers_components)
add_subdirectory(ch2_packaging_design_rules)
add_subdirectory(ch3_physical_design_factoring)

# Convenience target: build and run all demos
add_custom_target(run_all
    COMMAND ch0_software_capital
    COMMAND ch0_hierarchical_reuse
    COMMAND ch1_component_demo
    COMMAND ch1_linkage_demo
    COMMAND ch2_design_rules_demo
    COMMAND ch3_levelization_demo
    DEPENDS
        ch0_software_capital
        ch0_hierarchical_reuse
        ch1_component_demo
        ch1_linkage_demo
        ch2_design_rules_demo
        ch3_levelization_demo
    COMMENT "Running all chapter demos"
)
