cmake_minimum_required(VERSION 3.8.2)

project(MINI_DATA_CAPTURE)

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)

# There may be a way to tell up front if Qt5 is going to be found, but I haven't found
# a foolproof way to do it yet, so settle for the default error message for now.
#if(NOT CMAKE_PREFIX_PATH AND NOT Qt5Widgets_DIR)
#    message(WARNING "CMAKE_PREFIX_PATH is not defined, so find_package may not work. Set the CMAKE_PREFIX_PATH "
#            "environment variable to the install prefix of Qt 5, either on the command line as "
#            "-DCMAKE_PREFIX_PATH=\"path/to/Qt5/lib/cmake\" or with set(CMAKE_PREFIX_PATH path/to/Qt5/lib/cmake)")
#endif(NOT CMAKE_PREFIX_PATH AND NOT Qt5Widgets_DIR)

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

include_directories(./source)
find_package(Qt5 COMPONENTS Core Widgets Quick PrintSupport Bluetooth REQUIRED)

set (SOURCES
    app/main.cpp
    app/Application.cpp
    app/FileUpload.cpp
    app/FileDecoder.cpp
        )

set(HEADERS
        app/Application.h
        app/FileUpload.h
        app/FileDecoder.h
        )


set (UIS
    app/application.ui
        )

set ( RESOURCES
 qml.qrc
)

# Generate code from ui files
qt5_wrap_ui(UI_HEADERS ${UIS})

# Generate rules for building app files from the resources
qt5_add_resources(QRCS ${RESOURCES})

# Tell CMake to create the helloworld executable
add_executable(SMART ${UI_HEADERS} ${QRCS} ${SOURCES} ${HEADERS})

# Add the Qt5 Widgets for linking
target_link_libraries(SMART Qt5::Gui Qt5::Widgets Qt5::Core Qt5::Quick Qt5::PrintSupport)
