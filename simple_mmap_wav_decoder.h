
//
// Created by William.Hua on 2021/2/5.
//

#ifndef LOW_LATENCY_AUDIO_DECODE__SIMPLE_MMAP_WAV_DECODER_H
#define LOW_LATENCY_AUDIO_DECODE__SIMPLE_MMAP_WAV_DECODER_H

#include "dr_wav.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <iostream>
using namespace std;
class SimpleMMapWavDecoder
{
public:
    bool openWavFile(const std::string& infile){
        int data_pos = readWavHeaderAndDataPos(infile);
        openAndMmap(infile);

        // skip header
        map_ += data_pos;

        return true;
    }

    int readWavHeaderAndDataPos(const std::string& infile){
        // open file
        drwav  wav;
        if(!drwav_init_file(&wav, infile.c_str(), NULL)){
            printf("Failed to open file\n");
            exit(EXIT_FAILURE);
        }

        // only support stereo float 32 wav format
        if(wav.channels != 2){
            exit(EXIT_FAILURE);
        }

        if(wav.sampleRate != 44100){
            exit(EXIT_FAILURE);
        }

        if(wav.bitsPerSample != 32){
            exit(EXIT_FAILURE);
        }

        num_channels = wav.channels;
        sample_rate = wav.sampleRate;
        total_frames = wav.totalPCMFrameCount;
        int data_pos = wav.dataChunkDataPos;

        drwav_uninit(&wav);
        return data_pos;
    }

    void openAndMmap(const std::string& infile){
        fd_ = open(infile.c_str(), O_RDONLY, (mode_t)(0600));
        if(fd_ == -1)
        {
            cerr << "Error open file" << endl;
            exit(EXIT_FAILURE);
        }

        if(fstat(fd_, &file_info_) == -1)
        {
            cerr << "Error getting the file size" << endl;
            exit(EXIT_FAILURE);
        }

        if(file_info_.st_size == 0)
        {
            cerr << "file is empty, nothing to do" << endl;
            exit(EXIT_FAILURE);
        }

        cout << "file size:" << file_info_.st_size << endl;

        map_ = (char*)mmap(0, file_info_.st_size, PROT_READ, MAP_SHARED, fd_, 0);
        if(map_ == MAP_FAILED){
            close(fd_);
            cerr << "error mmapping the file" << endl;
            exit(EXIT_FAILURE);
        }
    }

    int fd_{-1};
    struct stat file_info_;
    char* map_;
    int current_frame{0};
    int num_channels{0};
    int sample_rate{0};
    int total_frames{0};
};

#endif //LOW_LATENCY_AUDIO_DECODE__SIMPLE_MMAP_WAV_DECODER_H
