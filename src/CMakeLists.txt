file(GLOB files "*.cpp")
include_directories(${Msync_INCLUDE_DIR})
#add_executable(msyncd ${files})
add_library(msync SHARED ${files})
