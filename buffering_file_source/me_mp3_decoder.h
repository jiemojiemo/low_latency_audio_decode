//
// Created by hw on 2019/11/15.
//

#pragma once
#include "me_audio_decoder.h"

namespace mammon
{
class MP3Decoder : public AudioDecoder
{
public:
    explicit MP3Decoder();
    virtual ~MP3Decoder() = default;

    bool open() override ;

    int open(const std::string &filename) override;

    void close() override;

    bool isOpen() override;

    bool seekToFrame(size_t frame_offset) override;
    std::vector<float> read(size_t size) override;
    size_t read(size_t size, float *output) override;

private:
    class Impl;
    std::shared_ptr<Impl> impl_;
};
}
