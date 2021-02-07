
//
// Created by William.Hua on 2021/2/2.
//

#pragma once
#include "me_file_source.h"
#include "me_time_slice_thread.h"
#include <list>
#include <mutex>
namespace mammon
{
class BufferingFileSource : public FileSource,
                            public TimeSliceClient
{
public:
    explicit BufferingFileSource(std::unique_ptr<FileSource> file_src,
                                 TimeSliceThread& time_slice_thread,
                                 int sample_to_buffer);

    ~BufferingFileSource() override = default;

    bool seek(size_t position) override;

    size_t read(float *buffer, size_t frame_num) override;
    size_t getNumChannel() const override;
    size_t getSampleRate() const override;
    size_t getNumFrames() const override;
    size_t getNumBit() const override;
    size_t getPosition() override;

    FileSource* getFileSource() const;

    int useTimeSlice() override;

    int getNumBlock() const;

    int getNumSamplePerBlock() const;

    const TimeSliceThread& getThread() const;

    class BufferBlock
    {
    public:
        BufferBlock(FileSource* file_src, size_t pos, int num_frames)
            : range(pos, pos + num_frames),
              buffer(file_src->getNumChannel() * num_frames)
        {
            if(file_src->getPosition() != pos){
                file_src->seek(pos);
            }

            file_src->read(buffer.data(), num_frames);
        }

        bool intersects(const std::pair<int64_t , int64_t>& other_range) const{
            return other_range.first < range.second || other_range.second < range.second;
        }

        bool contains(int64_t pos){
            return pos >= range.first && pos < range.second;
        }

        std::pair<int64_t , int64_t> range;
        std::vector<float> buffer;
    };

private:
    BufferBlock* getBlockContaining(size_t pos);

    bool readNextBufferChunk();

private:
    static constexpr int kSamplePerBlock = 32768;
    size_t num_blocks_to_hold_buffer_{0};
    std::unique_ptr<FileSource> file_src_{nullptr};
    std::atomic<size_t> pos_{0};
    std::list<std::shared_ptr<BufferBlock>> blocks_;
    mammon::TimeSliceThread& t_;
    std::mutex mutex_blocks_;
    int timeout_ms_{10};
};
}
