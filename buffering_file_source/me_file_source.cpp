//
//  ae_file_source.cpp
//
//  Created by tangshi on 2019/11/19.
//

#include "me_file_source.h"
#include "me_audio_decoder.h"
#include "me_buffering_file_source.h"
#include "me_time_slice_thread.h"

#include "me_wav_decoder.h"

using namespace std;

namespace mammon
{

class FileSourceImpl : public FileSource
{
public:

    FileSourceImpl(unique_ptr<AudioDecoder> decoder, string path)
        : decoder_(std::move(decoder)), path_(std::move(path)) {
        decoder_->open(path_);

        if(!decoder_->isOpen())
        {
            printf ("[ERROR] failed to open %s", path_.c_str());
        }
    }

    bool seek (size_t position) override
    {
        bool is_ok = decoder_->seekToFrame (position);
        if (is_ok)
        {
            pos_ = position;
        }

        return is_ok;
    }

    size_t read (float * buffer, size_t frame_num) override
    {
        auto num_frameread =
            decoder_->read (frame_num * decoder_->numChannels(), buffer) /
            decoder_->numChannels();

        pos_ += num_frameread;

        return num_frameread;  // return number of frames
    }

    size_t getNumChannel() const override
    {
        return static_cast<size_t> (decoder_->numChannels());
    }

    size_t getSampleRate() const override
    {
        return static_cast<size_t> (decoder_->sampleRate());
    }

    size_t getNumFrames() const override
    {
        return static_cast<size_t> (decoder_->numFrames());
    }

    size_t getNumBit() const override
    {
        return static_cast<size_t> (decoder_->numBit());
    }

    string getPath() const override
    {
        return path_;
    }

    size_t getPosition() override
    {
        return pos_;
    }

    bool openedOk()
    {
        return decoder_->isOpen();
    }

private:
    unique_ptr<AudioDecoder> decoder_;
    string path_;
    size_t pos_ {0};
};

std::unique_ptr<FileSource> FileSource::create (const std::string & path)
{
    size_t pos = path.rfind ('.');
    string ext;
    if (pos != path.length())
    {
        ext = path.substr (pos + 1);
    }
    else
    {
        return nullptr;
    }

    for (auto & c : ext)
    {
        c = std::tolower (c);
    }

    std::unique_ptr<FileSourceImpl> file_source;

    if (ext == "wav") {
        file_source =  make_unique<FileSourceImpl>(make_unique<WavDecoder>(), path);
    } else if (ext == "mp3") {
//        file_source =  make_unique<FileSourceImpl>(make_unique<MP3Decoder>(), path);
    } else{
        return nullptr;
    }

    if (!file_source->openedOk())
    {
        return nullptr;
    }

    return file_source;
}


std::unique_ptr<FileSource> FileSource::createBufferingFileSource(const string &path,
                                                                  TimeSliceThread &t,
                                                                  int sample_to_buffer) {
    auto file_src = FileSource::create(path);
    if(file_src != nullptr){
        auto b_file_src = std::make_unique<BufferingFileSource>(std::move(file_src), t, sample_to_buffer);
        return b_file_src;
    }else{
        return nullptr;
    }
}
}
