cmake_minimum_required(VERSION 2.8)
project(bmx2ogg)



# for iconv
add_definitions(-DLIBICONV_PLUG)
add_definitions(-std=c++11)
add_subdirectory(bmsbelplus)
add_executable(bmx2ogg
	"bmx2ogg/bmx2wav_main.cpp"
	"bmx2ogg/bmx2wav_common.cpp"
	"bmx2ogg/audio.cpp"
	"bmx2ogg/exception.cpp"
)



#
# link: glibc sndfile
#


find_path(SNDFILE_INCLUDE_DIR
	NAMES
		sndfile.hh
	PATHS
		/usr/include
		/usr/local/include
)

find_library(SNDFILE_LIBRARY
	NAMES
		sndfile
	PATHS
		/lib
		/usr/lib
		/usr/local/lib
)

find_path(ICONV_INCLUDE_DIR
	NAMES
		iconv.h
	PATHS
		/usr/include
		/usr/local/include
)

find_library(ICONV_LIB
	NAMES
		iconv
	PATHS
		/lib
		/usr/lib
		/usr/local/lib
)

MESSAGE( STATUS "SNDFILE_INCLUDE_DIR" ${SNDFILE_INCLUDE_DIR} )
MESSAGE( STATUS "SNDFILE_LIBRARY" ${SNDFILE_LIBRARY} )
MESSAGE( STATUS "ICONV_INCLUDE_DIR" ${ICONV_INCLUDE_DIR} )
MESSAGE( STATUS "ICONV_LIB" ${ICONV_LIB} )

# including priority is important
include_directories(
	${SNDFILE_INCLUDE_DIR}
	${ICONV_INCLUDE_DIR}
)
include_directories(
	"bmsbelplus/include"
)

target_link_libraries(bmx2ogg
	${SNDFILE_LIBRARY}
	${ICONV_LIB}
	bmsbel
)

