//
// Created by hw on 2019/11/6.
//

#pragma once
#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace mammon
{
class AudioDecoder
{
public:
    virtual ~AudioDecoder() = default;

    /**
     * 必须在调用 read 函数前 使用 open()
     * @return true 表示打开文件成功，其他表示失败
     */
    virtual bool open() = 0;

    virtual int open(const std::string& filename) = 0;

    virtual void close() = 0;

    /**
     * Returns true if the file opened without problems.
     */
    virtual bool isOpen() = 0;

    /**
     * 移动到具体的采样点位置
     *
     * @param frame_offset 移动的距离,以音频帧为单位
     * @return true 表示移动成功 否则表示失败
     */
    virtual bool seekToFrame(size_t frame_offset) = 0;

    /**
     * 移动到对应时间位置
     *
     * @param second 移动距离,以秒为单位
     * @return true 表示移动成功 否则表示失败
     */
    bool seekToPosition(double second)
    {
        if(second < 0)
        {
            return false;
        }

        size_t frame_offset = static_cast<size_t >(second * sample_rate_);
        return seekToFrame(frame_offset);
    }


    /**
     * 从当前位置开始连续读取 size 个采样点，采用交织型的排列方式，并且采样点的值在[-1,1]之间
     *
     * 例如，读取 stereo 音频 1024 个数据，那么左右声道各 512 采样点；
     *
     * @param size 要读取的采样点数量
     * @return 采样点vector，vector中的采样点个数可能小于`size`
     */
    virtual std::vector<float> read(size_t size) = 0;

    /**
     * 从当前位置开始连续读取 size 个采样点到 output 中，采用交织型的排列方式，并且采样点的值在[-1,1]之间
     *
     * @param size 读取的采样点数量
     * @param output 指向存放数据内存的指针，确保其大小至少为 size
     * @return 实际读取到的采样点数据 读取的样本数目可能小于`size`
     */
    virtual size_t read(size_t size, float* output) = 0;

    inline uint32_t numChannels() const{return num_channels_;}

    inline uint64_t numFrames() const{ return num_frames_;}

    inline uint32_t sampleRate() const{return sample_rate_;}

    inline uint32_t numBit() const {return bit_depth_;}

    inline float duration() const{return num_frames_ /sample_rate_;}

protected:
    uint32_t num_channels_{0};
    uint32_t sample_rate_{0};
    uint32_t bit_depth_{0};
    uint64_t num_frames_{0};
    std::string filename_;
};

}
