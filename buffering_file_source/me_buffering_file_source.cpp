
//
// Created by William.Hua on 2021/2/2.
//

#include "me_buffering_file_source.h"
#include "me_time_slice_thread.h"
namespace mammon
{
BufferingFileSource::BufferingFileSource(std::unique_ptr<FileSource> file_src,
                                         TimeSliceThread& time_slice_thread,
                                         int sample_to_buffer):
    num_blocks_to_hold_buffer_(sample_to_buffer/kSamplePerBlock + 1),
    file_src_(std::move(file_src)),
    t_(time_slice_thread)
{
    for(int i = 0; i < 3; ++i){
        readNextBufferChunk();
    }

    t_.addTimeSliceClient(this);
}

bool BufferingFileSource::seek(size_t position) {
    if(position >= getNumFrames()){
        return false;
    }else{
        pos_ = position;
        return true;
    }
}

size_t BufferingFileSource::read(float *buffer, size_t frame_num)  {
    auto start_time = std::chrono::steady_clock::now();

    auto num_frame_need_to_read = frame_num;
    size_t num_actual_read = 0u;
    for(;num_actual_read < frame_num;)
    {
        auto cur_pos = pos_.load();
        {
            BufferBlock* block = nullptr;
            {
                std::lock_guard<std::mutex> lck(mutex_blocks_);
                block = getBlockContaining(cur_pos);
                if(block)
                {
                    auto num_to_read_frame = std::min(num_frame_need_to_read, (size_t)(block->range.second - cur_pos));
                    auto num_to_read_samples = num_to_read_frame * getNumChannel();
                    auto offset_samples = (cur_pos - block->range.first) * getNumChannel();

                    std::copy_n(block->buffer.data() + offset_samples, num_to_read_samples, buffer);

                    pos_ += num_to_read_frame;
                    num_actual_read += num_to_read_frame;
                    num_frame_need_to_read -= num_to_read_frame;
                }
            }

            if(block == nullptr)
            {
                if(timeout_ms_ >= 0 && std::chrono::steady_clock::now() >= start_time + std::chrono::milliseconds{timeout_ms_})
                {
                    auto num_left_samples_to_fill =(frame_num - num_actual_read) * getNumChannel();
                    auto offset_samples = num_actual_read * getNumChannel();
                    std::fill_n (buffer + offset_samples,num_left_samples_to_fill,0.0f);

                    // return num frame of filled
                    return (frame_num - num_actual_read);
                }else
                {
                    std::this_thread::yield();
                }
            }
        }
    }

    return num_actual_read;
}
size_t BufferingFileSource::getNumChannel() const  {
    return file_src_->getNumChannel();
}
size_t BufferingFileSource::getSampleRate() const  {
    return file_src_->getSampleRate();
}
size_t BufferingFileSource::getNumFrames() const  {
    return file_src_->getNumFrames();
}
size_t BufferingFileSource::getNumBit() const  {
    return file_src_->getNumBit();
}
size_t BufferingFileSource::getPosition()  {
    return pos_.load();
}

FileSource* BufferingFileSource::getFileSource() const{
    return file_src_.get();
}

int BufferingFileSource::useTimeSlice()  {
    return readNextBufferChunk() ? 1 : 100;
}

int BufferingFileSource::getNumBlock() const{
    return num_blocks_to_hold_buffer_;
}

int BufferingFileSource::getNumSamplePerBlock() const{
    return kSamplePerBlock;
}

const TimeSliceThread& BufferingFileSource::getThread() const{
    return t_;
}

bool BufferingFileSource::readNextBufferChunk(){
    auto pos = pos_.load();
    auto start_pos = (pos / kSamplePerBlock) * kSamplePerBlock;
    auto end_pos = start_pos + num_blocks_to_hold_buffer_ * kSamplePerBlock;

    std::list<std::shared_ptr<BufferBlock>> new_blocks;
    for(auto iter = blocks_.begin(); iter != blocks_.end(); ++iter){
        if((*iter)->intersects({start_pos, end_pos})){
            new_blocks.push_back(*iter);
        }
    }

    if(new_blocks.size() == num_blocks_to_hold_buffer_){
        return false;
    }

    for(auto p = start_pos; p < end_pos; p+=kSamplePerBlock)
    {
        if(getBlockContaining(p) == nullptr)
        {
            new_blocks.emplace_back(new BufferBlock(file_src_.get(), pos, kSamplePerBlock));
            break; // just do one block
        }
    }

    // need a lock
    {
        std::lock_guard<std::mutex> l(mutex_blocks_);
        new_blocks.swap(blocks_);
    }


    return true;
}

BufferingFileSource::BufferBlock* BufferingFileSource::getBlockContaining(size_t pos){
    BufferBlock* block = nullptr;
    for(const auto& item : blocks_){
        if(item->contains(pos)){
            block = item.get();
            break;
        }
    }
    return block;
}

}