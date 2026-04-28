cmake_minimum_required(VERSION 3.16)
project(ch3_physical_design_factoring LANGUAGES CXX)

# ============================================================
# LEVELIZATION TECHNIQUES
# ============================================================

# --- Opaque Pointer (PIMPL) ---
add_library(pimpl_device STATIC
    src/levelization/opaque_pointer/device.cpp
)
target_include_directories(pimpl_device PUBLIC src/levelization/opaque_pointer)
target_compile_features(pimpl_device PRIVATE cxx_std_17)

add_executable(ch3_opaque_pointer_demo
    src/levelization/opaque_pointer/opaque_pointer_demo.cpp
)
target_link_libraries(ch3_opaque_pointer_demo PRIVATE pimpl_device)
target_include_directories(ch3_opaque_pointer_demo PRIVATE src/levelization/opaque_pointer)
target_compile_features(ch3_opaque_pointer_demo PRIVATE cxx_std_17)

# --- Dumb Data ---
add_library(dumb_data_lib STATIC
    src/levelization/dumb_data/node.cpp
    src/levelization/dumb_data/graph.cpp
)
target_include_directories(dumb_data_lib PUBLIC src/levelization/dumb_data)
target_compile_features(dumb_data_lib PRIVATE cxx_std_17)

add_executable(ch3_dumb_data_demo
    src/levelization/dumb_data/dumb_data_demo.cpp
)
target_link_libraries(ch3_dumb_data_demo PRIVATE dumb_data_lib)
target_include_directories(ch3_dumb_data_demo PRIVATE src/levelization/dumb_data)
target_compile_features(ch3_dumb_data_demo PRIVATE cxx_std_17)

# --- Callbacks ---
add_library(callbacks_lib STATIC
    src/levelization/callbacks/event_queue.cpp
    src/levelization/callbacks/logger.cpp
)
target_include_directories(callbacks_lib PUBLIC src/levelization/callbacks)
target_compile_features(callbacks_lib PRIVATE cxx_std_17)

add_executable(ch3_callbacks_demo
    src/levelization/callbacks/callbacks_demo.cpp
)
target_link_libraries(ch3_callbacks_demo PRIVATE callbacks_lib)
target_include_directories(ch3_callbacks_demo PRIVATE src/levelization/callbacks)
target_compile_features(ch3_callbacks_demo PRIVATE cxx_std_17)

# --- Protocol Class ---
add_library(protocol_lib STATIC
    src/levelization/protocol_class/file_persistence.cpp
    src/levelization/protocol_class/memory_persistence.cpp
    src/levelization/protocol_class/data_store.cpp
)
target_include_directories(protocol_lib PUBLIC src/levelization/protocol_class)
target_compile_features(protocol_lib PRIVATE cxx_std_17)

add_executable(ch3_protocol_demo
    src/levelization/protocol_class/protocol_demo.cpp
)
target_link_libraries(ch3_protocol_demo PRIVATE protocol_lib)
target_include_directories(ch3_protocol_demo PRIVATE src/levelization/protocol_class)
target_compile_features(ch3_protocol_demo PRIVATE cxx_std_17)

# --- Demotion ---
add_executable(ch3_demotion_demo
    src/levelization/demotion/demotion_demo.cpp
)
target_compile_features(ch3_demotion_demo PRIVATE cxx_std_17)

# --- Main levelization demo (combines all techniques) ---
add_executable(ch3_levelization_demo
    src/levelization_demo.cpp
)
target_compile_features(ch3_levelization_demo PRIVATE cxx_std_17)

# ============================================================
# INSULATION TECHNIQUES
# ============================================================

# --- Compile-time coupling ---
add_library(heavy_widget STATIC
    src/insulation/compile_time_coupling/heavy_widget.cpp
)
target_include_directories(heavy_widget PUBLIC src/insulation/compile_time_coupling)
target_compile_features(heavy_widget PRIVATE cxx_std_17)

add_library(insulated_widget STATIC
    src/insulation/pimpl_insulation/insulated_widget.cpp
)
target_include_directories(insulated_widget PUBLIC src/insulation/pimpl_insulation)
target_compile_features(insulated_widget PRIVATE cxx_std_17)

add_executable(ch3_insulation_demo
    src/insulation/insulation_demo.cpp
)
target_link_libraries(ch3_insulation_demo PRIVATE heavy_widget insulated_widget)
target_include_directories(ch3_insulation_demo PRIVATE
    src/insulation/compile_time_coupling
    src/insulation/pimpl_insulation
)
target_compile_features(ch3_insulation_demo PRIVATE cxx_std_17)
