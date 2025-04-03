cmake_minimum_required(VERSION 3.10)
project(tftp_client C)

# Find GTK3 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Include directories
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

# Add definitions
add_definitions(${GTK3_CFLAGS_OTHER})

# Set Windows-specific properties to hide console window
if(WIN32)
  set(GUI_TYPE WIN32)
endif()

# Add the executable with GUI_TYPE flag to hide console on Windows
add_executable(tftp_client ${GUI_TYPE} 
  main.c 
  tftp_client.c
  tftp_gui.c
)

target_link_libraries(tftp_client ${GTK3_LIBRARIES})

# Add Windows-specific libraries if on Windows
if(WIN32)
    target_link_libraries(tftp_client ws2_32)
endif()
