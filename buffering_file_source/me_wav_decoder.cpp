//
// Created by hw on 2019/11/6.
//
#include <cinttypes>
#include "me_wav_decoder.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include <cassert>

namespace mammon
{

class WavDecoder::Impl
{
public:
    Impl(WavDecoder* parent):
        wav_(nullptr),
        parent_(parent)

    {

    }

    ~Impl()
    {
        close();
    }

    int open(const std::string& wav_path)
    {
        if(wav_ == nullptr)
        {
            wav_ = new drwav();
        }

        if(!drwav_init_file(wav_, wav_path.c_str(), NULL))
        {
            printf("[ERROR][WavDecoder]cannot open %s\n", wav_path.c_str());

            delete(wav_);
            wav_ = nullptr;

            return -1;
        }else
        {
            parent_->num_channels_ = static_cast<uint32_t>(wav_->channels);
            parent_->sample_rate_  = wav_->sampleRate;
            parent_->bit_depth_    = static_cast<uint32_t>(wav_->bitsPerSample);
            parent_->num_frames_   = wav_->totalPCMFrameCount;

            printf("[ERROR][WavDecoder] Opened file: \n Channel=" PRIu32 " SampleRate=" PRIu32 " BitDepth=" PRIu32 " TotalFrames=" PRIu64 "\n",
                    parent_->num_channels_, parent_->sample_rate_, parent_->bit_depth_, parent_->num_frames_);

            return 0;
        }
    }

    void close(){
        if(wav_!= nullptr)
        {
            drwav_uninit(wav_);

            delete(wav_);
            wav_ = nullptr;

            parent_->num_channels_  = 0;
            parent_->sample_rate_   = 0;
            parent_->bit_depth_     = 0;
            parent_->num_frames_    = 0;
            parent_->filename_      = "";
        }
    }

    bool isOpen() const
    {
        return wav_ != nullptr;
    }

    bool seekToFrame(size_t frame_offset) const
    {
        if(frame_offset >= wav_->totalPCMFrameCount)
        {
            // printf("[ERROR][WavDecoder]seek error: out of bound %d beyond %d\n", frame_offset, static_cast<int>(wav_->totalPCMFrameCount));
            return false;
        }

        return drwav_seek_to_pcm_frame(wav_, frame_offset);
    }

    size_t read(size_t size, float* output) const
    {
        if(!isOpen()){
            return 0;
        }
        const size_t num_frame = size/wav_->channels;
        auto readed_frames = drwav_read_pcm_frames_f32(wav_, num_frame, output);
        return readed_frames*wav_->channels;
    }

    drwav* wav_;
    WavDecoder* parent_;
};

WavDecoder::WavDecoder():
      impl_(std::make_shared<Impl>(this))
{

}

bool WavDecoder::open()
{
    return impl_->open(filename_);
}

bool WavDecoder::seekToFrame(size_t frame_offset)
{
    return impl_->seekToFrame(frame_offset);
}

std::vector<float> WavDecoder::read(size_t size)
{
    std::vector<float> results(size);

    read(size, results.data());

    return results;
}
size_t WavDecoder::read(size_t size, float* output)
{
    auto num_read = impl_->read(size, output);
//    if(num_read != size)
//    {
//        printf("Need %zu sample, but only read %zu\n", size, num_read);
//    }
    return num_read;
}
bool WavDecoder::isOpen()
{
    return impl_->isOpen();
}

int WavDecoder::open(const std::string &filename) {
    filename_ = filename;
    return impl_->open(filename);
}


void WavDecoder::close() {
    impl_->close();
}

}
