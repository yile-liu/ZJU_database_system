#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glog::glog" for configuration "Debug"
set_property(TARGET glog::glog APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glog::glog PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libglogd.0.6.0.dylib"
  IMPORTED_SONAME_DEBUG "@rpath/libglogd.1.dylib"
  )

list(APPEND _cmake_import_check_targets glog::glog )
list(APPEND _cmake_import_check_files_for_glog::glog "${_IMPORT_PREFIX}/lib/libglogd.0.6.0.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
