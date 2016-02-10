cmake_minimum_required(VERSION 2.8)
project(bmx2ogg)
include_directories(
	"include"
	"bmsbelplus/include"
)
add_definitions(-std=c++11)
add_subdirectory(bmsbelplus)
add_executable(bmx2ogg
	"bmx2ogg/bmx2wav_main.cpp"
	"bmx2ogg/bmx2wav_buffer.cpp"
	"bmx2ogg/bmx2wav_common.cpp"
	"bmx2ogg/bmx2wav_ogg.cpp"
	"bmx2ogg/bmx2wav_wav.cpp"
	"bmx2ogg/bmx2wav_wav_maker.cpp"
	"bmx2ogg/bmx2wav_wav_maker_ogg.cpp"
	"bmx2ogg/exception.cpp"
)

#
# link: libogg/libvorbis, libiconv
#

#find_library(bmsbel NAMES bmsbel PATHS "bmsbelplus/build")
target_link_libraries(bmx2ogg
	vorbisfile
	vorbisenc
	vorbis
	ogg
	iconv
	bmsbel
)
