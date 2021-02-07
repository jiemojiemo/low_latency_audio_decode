add_library(buffering_file_source
        me_buffering_file_source.cpp
        me_time_slice_thread.cpp
        me_wav_decoder.cpp
        me_file_source.cpp
        )

target_include_directories(buffering_file_source PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(buffering_file_source PRIVATE dr_libs)

target_compile_features(buffering_file_source PRIVATE cxx_std_14)