cmake_minimum_required(VERSION 3.16)
project(ch2_packaging_design_rules LANGUAGES CXX)

# Package library: pkg (contains pkg_date + pkg_dateutil components)
add_library(pkg STATIC
    src/pkg/pkg_date.cpp
    src/pkg/pkg_dateutil.cpp
)
target_include_directories(pkg PUBLIC src)
target_compile_features(pkg PRIVATE cxx_std_17)

# Test driver for pkg_date component (hierarchical testability demo)
add_executable(ch2_pkg_date_test
    src/pkg/pkg_date.t.cpp
)
target_link_libraries(ch2_pkg_date_test PRIVATE pkg)
target_include_directories(ch2_pkg_date_test PRIVATE src)
target_compile_features(ch2_pkg_date_test PRIVATE cxx_std_17)

# Design rules demo (illustrates all Ch2 naming and packaging rules)
add_executable(ch2_design_rules_demo
    src/design_rules_demo.cpp
)
target_link_libraries(ch2_design_rules_demo PRIVATE pkg)
target_include_directories(ch2_design_rules_demo PRIVATE src)
target_compile_features(ch2_design_rules_demo PRIVATE cxx_std_17)
