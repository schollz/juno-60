cmake_minimum_required(VERSION 3.5)
project(Juno60)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SC_PATH from environment or default
if(NOT DEFINED SC_PATH)
    if(DEFINED ENV{SC_PATH})
        set(SC_PATH $ENV{SC_PATH})
    else()
        set(SC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/supercollider")
    endif()
endif()

message(STATUS "SuperCollider path: ${SC_PATH}")

# Include SuperCollider plugin API
include_directories(
    ${SC_PATH}/include/plugin_interface
    ${SC_PATH}/include/common
    ${SC_PATH}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Find SuperCollider headers
if(EXISTS "${SC_PATH}/include/plugin_interface/SC_PlugIn.hpp")
    message(STATUS "Found SuperCollider plugin interface")
else()
    message(WARNING "SuperCollider plugin interface not found at ${SC_PATH}/include/plugin_interface/SC_PlugIn.hpp")
    message(STATUS "Make sure SC_PATH points to a valid SuperCollider source directory")
endif()

# Build the plugin
add_library(Juno60 MODULE
    src/Juno60.cpp
)

# Set output name
set_target_properties(Juno60 PROPERTIES
    PREFIX ""
    OUTPUT_NAME "Juno60"
)

# Platform-specific settings
if(APPLE)
    set_target_properties(Juno60 PROPERTIES
        SUFFIX ".scx"
        BUNDLE TRUE
    )
    target_link_libraries(Juno60 "-framework CoreFoundation")
elseif(WIN32)
    set_target_properties(Juno60 PROPERTIES
        SUFFIX ".scx"
    )
else()
    set_target_properties(Juno60 PROPERTIES
        SUFFIX ".so"
    )
endif()

# Installation
# Determine installation destination
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Use default user Extensions directory
    if(APPLE)
        set(EXTENSION_DIR "$ENV{HOME}/Library/Application Support/SuperCollider/Extensions")
    elseif(WIN32)
        set(EXTENSION_DIR "$ENV{LOCALAPPDATA}/SuperCollider/Extensions")
    else()
        set(EXTENSION_DIR "$ENV{HOME}/.local/share/SuperCollider/Extensions")
    endif()
    set(INSTALL_DEST "${EXTENSION_DIR}/Juno60")
else()
    # Respect CMAKE_INSTALL_PREFIX when explicitly set
    set(INSTALL_DEST "Juno60")
endif()

install(TARGETS Juno60
    DESTINATION "${INSTALL_DEST}"
)

install(FILES Juno60.sc
    DESTINATION "${INSTALL_DEST}"
)

# Custom uninstall target
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${EXTENSION_DIR}/Juno60"
    COMMENT "Uninstalling Juno60 plugin"
)
