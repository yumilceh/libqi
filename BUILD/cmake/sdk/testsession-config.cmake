# This is an autogenerated file. Do not edit

get_filename_component(_cur_dir ${CMAKE_CURRENT_LIST_FILE} PATH)
set(_root_dir "${_cur_dir}/../../../")
get_filename_component(ROOT_DIR ${_root_dir} ABSOLUTE)

 
set(TESTSESSION_INCLUDE_DIRS "${ROOT_DIR}/include;" CACHE STRING "" FORCE)
mark_as_advanced(TESTSESSION_INCLUDE_DIRS)
   
set(TESTSESSION_LIBRARIES
  ${ROOT_DIR}/lib/libtestsession.a
  CACHE STRING "" FORCE)

mark_as_advanced(TESTSESSION_LIBRARIES)
 
set(TESTSESSION_PACKAGE_FOUND TRUE CACHE INTERNAL "" FORCE)
 
set(TESTSESSION_DEPENDS "QI;BOOST;BOOST_ATOMIC;BOOST_DATE_TIME;BOOST_FILESYSTEM;BOOST_REGEX;BOOST_PROGRAM_OPTIONS;OPENSSL;BOOST_LOCALE;BOOST_THREAD;BOOST_CHRONO;BOOST_SYSTEM;PTHREAD;DL;RT" CACHE INTERNAL "" FORCE)
 