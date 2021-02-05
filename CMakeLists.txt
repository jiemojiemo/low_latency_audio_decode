cmake_minimum_required(VERSION 3.10)

project(low_latency_audio_decode)

add_subdirectory(portaudio EXCLUDE_FROM_ALL)
add_subdirectory(dr_libs)


add_executable(0_playback 0_playback.cpp)
target_link_libraries(0_playback portaudio dr_libs)
target_compile_features(0_playback PRIVATE cxx_std_14)