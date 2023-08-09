# Install script for directory: /Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/thirdparty/glog

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/libglogd.0.6.0.dylib"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/libglogd.1.dylib"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libglogd.0.6.0.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libglogd.1.dylib"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      execute_process(COMMAND /usr/bin/install_name_tool
        -delete_rpath "/opt/homebrew/lib"
        "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -x "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/libglogd.dylib")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libglogd.dylib" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libglogd.dylib")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/opt/homebrew/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libglogd.dylib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -x "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libglogd.dylib")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/glog" TYPE FILE FILES
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/glog/export.h"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/glog/logging.h"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/glog/raw_logging.h"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/glog/stl_logging.h"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/glog/vlog_is_on.h"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/thirdparty/glog/src/glog/log_severity.h"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/thirdparty/glog/src/glog/platform.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/libglog.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Development" OR NOT CMAKE_INSTALL_COMPONENT)
  
set (glog_FULL_CMake_DATADIR "\${CMAKE_CURRENT_LIST_DIR}/../../../share/glog/cmake")
set (glog_DATADIR_DESTINATION lib/cmake/glog)

if (NOT IS_ABSOLUTE lib/cmake/glog)
  set (glog_DATADIR_DESTINATION "${CMAKE_INSTALL_PREFIX}/${glog_DATADIR_DESTINATION}")
endif (NOT IS_ABSOLUTE lib/cmake/glog)

configure_file ("/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/thirdparty/glog/glog-modules.cmake.in"
  "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/CMakeFiles/CMakeTmp/glog-modules.cmake" @ONLY)
file (INSTALL
  "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/CMakeFiles/CMakeTmp/glog-modules.cmake"
  DESTINATION
  "${glog_DATADIR_DESTINATION}")

endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/glog" TYPE FILE FILES
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/glog-config.cmake"
    "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/glog-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/glog" TYPE DIRECTORY FILES "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/share/glog/cmake" FILES_MATCHING REGEX "/[^/]*\\.cmake$")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/glog/glog-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/glog/glog-targets.cmake"
         "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/CMakeFiles/Export/3b58dfe2d8c90c3a489d71c8991c4dd3/glog-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/glog/glog-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/glog/glog-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/glog" TYPE FILE FILES "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/CMakeFiles/Export/3b58dfe2d8c90c3a489d71c8991c4dd3/glog-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/glog" TYPE FILE FILES "/Users/yileliu/Desktop/浙江大学/2021-2022学年/2021春夏数据库系统/minisql/cmake-build-debug/glog-build/CMakeFiles/Export/3b58dfe2d8c90c3a489d71c8991c4dd3/glog-targets-debug.cmake")
  endif()
endif()

