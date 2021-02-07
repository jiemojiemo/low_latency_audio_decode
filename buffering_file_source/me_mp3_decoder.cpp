//
// Created by hw on 2019/11/15.
//

#include <cinttypes>
#include "me_mp3_decoder.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#include <vector>

namespace mammon
{

class MP3Decoder::Impl
{
public:
    Impl(MP3Decoder* parent)
        : parent_(parent),
          mp3_(nullptr)
    {

    }

    ~Impl()
    {
        close();
    }

    void close() {
        if (mp3_ != nullptr) {
            drmp3_uninit(mp3_);

            delete (mp3_);
            mp3_ = nullptr;

            parent_->num_channels_  = 0;
            parent_->sample_rate_   = 0;
            parent_->bit_depth_     = 0;
            parent_->num_frames_    = 0;
            parent_->filename_      = "";
        }
    }

    int open(const std::string& mp3_path) {
        if(mp3_ == nullptr)
        {
            mp3_ = new drmp3();
        }

        if (!drmp3_init_file(mp3_, mp3_path.c_str(), NULL))
        {
            printf("[ERROR][MP3Decoder]cannot open %s\n", mp3_path.c_str());

            delete(mp3_);
            mp3_ = nullptr;

            return -1;
        }else
        {
            parent_->num_channels_ = mp3_->channels;
            parent_->sample_rate_ = mp3_->sampleRate;
            parent_->bit_depth_ = 16; // mp3 bit depth default is 16
            parent_->num_frames_ = drmp3_get_pcm_frame_count(mp3_);

            printf("[INFO][MP3Decoder] Opened file: \n Channel=" PRIu32 " SampleRate=" PRIu32 " BitDepth=" PRIu32 " TotalFrames=" PRIu64 "\n",
                   parent_->num_channels_, parent_->sample_rate_, parent_->bit_depth_, parent_->num_frames_);

            return 0;
        }
    }

    bool isOpen() const
    {
        return mp3_ != nullptr;
    }

    bool seekToFrame(size_t frame_offset) const
    {
        if(frame_offset >= parent_->num_frames_)
        {
            printf("[ERROR][MP3Decoder]seek error: out of bound\n");
            return false;
        }

        return drmp3_seek_to_pcm_frame(mp3_, frame_offset);
    }

    size_t read(size_t size, float* output) const
    {
        if(!isOpen()){
            return 0;
        }

        const size_t num_frame = size/mp3_->channels;
        auto readed_frames = drmp3_read_pcm_frames_f32(mp3_, num_frame, output);
        return readed_frames*mp3_->channels;
    }

    MP3Decoder* parent_;
    drmp3* mp3_;
};

MP3Decoder::MP3Decoder():
    impl_(std::make_shared<Impl>(this))
{

}

bool MP3Decoder::open()
{
    return impl_->open(filename_);
}

bool MP3Decoder::seekToFrame(size_t frame_offset)
{
    return impl_->seekToFrame(frame_offset);
}

std::vector<float> MP3Decoder::read(size_t size)
{
    std::vector<float> results(size);

    read(size, results.data());

    return results;
}

size_t MP3Decoder::read(size_t size, float *output)
{
    auto num_read = impl_->read(size, output);
//    if(num_read != size)
//    {
//        printf("[INFO][MP3Decoder]Need %zu sample, but only read %zu\n", size, num_read);
//    }
    return num_read;
}
bool MP3Decoder::isOpen()
{
    return impl_->isOpen();
}
int MP3Decoder::open(const std::string &filename) {
    filename_ = filename;
    return impl_->open(filename);
}


void MP3Decoder::close() {
    impl_->close();
}

}
