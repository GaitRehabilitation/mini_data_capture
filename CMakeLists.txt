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
if (WIN32)
    set(CMAKE_WIN32_EXECUTABLE ON)
endif ()

include_directories(./source)
find_package(Qt5 COMPONENTS Core Widgets Quick PrintSupport Bluetooth REQUIRED)

set(SOURCES
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


set(UIS
        app/application.ui
        )

set(RESOURCES
        qml.qrc
        )

# Generate code from ui files
qt5_wrap_ui(UI_HEADERS ${UIS})

# Generate rules for building app files from the resources
qt5_add_resources(QRCS ${RESOURCES})

# Tell CMake to create the helloworld executable
add_executable(MINI_DECODER ${UI_HEADERS} ${QRCS} ${SOURCES} ${HEADERS})

# Add the Qt5 Widgets for linking
target_link_libraries(MINI_DECODER Qt5::Gui Qt5::Widgets Qt5::Core Qt5::Quick Qt5::PrintSupport)



install(TARGETS MINI_DECODER
        DESTINATION "."
        )



if (MSVC)
    get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(
            TARGET SMART POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/windeployqt_stuff
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --compiler-runtime --dir ${CMAKE_BINARY_DIR}/windeployqt_stuff $<TARGET_FILE:SMART>
    )

    add_custom_command(
            TARGET SMART POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --compiler-runtime $<TARGET_FILE:SMART>
    )

    install(
            DIRECTORY ${CMAKE_BINARY_DIR}/windeployqt_stuff/
            DESTINATION .
    )

    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ./)
    include(InstallRequiredSystemLibraries)
endif(MSVC)



set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Mini Data Capture Decoder Application")

if(NOT DEFINED ${CPACK_PACKAGE_VERSION_MAJOR})
    SET(CPACK_PACKAGE_VERSION_MAJOR 0)
endif()

if(NOT DEFINED ${CPACK_PACKAGE_VERSION_MINOR})
    SET(CPACK_PACKAGE_VERSION_MINOR 0)
endif()

if(NOT DEFINED ${CPACK_PACKAGE_VERSION_PATCH})
    SET(CPACK_PACKAGE_VERSION_PATCH 0)
endif()

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME "Mini Data Decoder")
set(CPACK_PACKAGE_VENDOR "GrailLab")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Utility for collecting data from Mbient sensors")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MINI_DECODER")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")


set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
set(CPACK_PACKAGE_EXECUTABLES "MINI_DECODER" "MINI_DECODER")
set(CPACK_CREATE_DESKTOP_LINKS "MINI_DECODER")
set(CPACK_NSIS_MODIFY_PATH "ON")

#set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/icons\\\\gait-icon.bmp")
#set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/icons/gait-icon.ico")
#set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/icons/gait-icon.ico")

set (CPACK_NSIS_MODIFY_PATH "ON")
INCLUDE(CPack)