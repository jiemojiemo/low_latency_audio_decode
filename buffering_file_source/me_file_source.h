#pragma once

#ifndef _MAMMON_AE_FILE_SOURCE_HEADER_
#define _MAMMON_AE_FILE_SOURCE_HEADER_

#include <cstddef>
#include <string>
#include <memory>

namespace mammon {

class TimeSliceThread;

/**
 * @brief 文件数据源
 * 背后会对接解码器，解码出PCM流
 */
class FileSource {
public:
    /**
     * @brief 跳转流到指定位置
     * 不能成功seek时返回false
     * @param position 跳转的位置
     * @return true
     * @return false
     */
    virtual bool seek(size_t position) = 0;
    /**
     * @brief 从流中读取数据
     * 成功读取时，总的写入数据量是 num_frame * num_channel
     * 每次读取后，流内的位置应该向后自动增加read的量
     * @param buffer 接收数据的缓冲区
     * @param frame_num 帧数
     * @return size_t 实际读取到的帧数
     */
    virtual size_t read(float* buffer, size_t frame_num) = 0;
    /**
     * @brief 获取通道数
     * 
     * @return size_t 
     */
    virtual size_t getNumChannel() const = 0;
    /**
     * @brief 获取采样率
     * 
     * @return size_t 
     */
    virtual size_t getSampleRate() const = 0;
    /**
     * @brief 获取总帧数
     * 
     * @return size_t 
     */
    virtual size_t getNumFrames() const = 0;
    /**
     * @brief 获取位深
     * 
     * @return size_t 
     */
    virtual size_t getNumBit() const  = 0;

    /**
     * @brief 获得当前解码位置
     */
    virtual size_t getPosition() = 0;

    virtual ~FileSource() = default;

    /**
     * @brief 从指定位置创建文件数据源
     * 通常会根据扩展名选择相应的解码器
     * @param path source file path
     * @return std::unique_ptr<FileSource>
     */
    static std::unique_ptr<FileSource> create(const std::string& path);

    /**
     * @brief create buffering file source
     * @param path source file path
     */
    static std::unique_ptr<FileSource> createBufferingFileSource(const std::string& path,
                                                                 TimeSliceThread& t,
                                                                 int sample_to_buffer);


    virtual std::string getPath() const {
        return "";
    }
};

} // namespace mammon

#endif
