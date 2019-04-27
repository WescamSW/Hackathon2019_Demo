/*
 * VideoOpenCV.h
 *
 *  Created on: Apr 15, 2019
 *      Author: slascos
 */

#ifndef SRC_VIDEOOPENCV_H_
#define SRC_VIDEOOPENCV_H_

#include <memory>
#include <thread>
#include <opencv2/core.hpp>
#include "wscDrone/VideoFrame.h"

class VideoFrameOpenCV : public VideoFrame {
public:
    VideoFrameOpenCV() = delete;
    VideoFrameOpenCV(unsigned height, unsigned width);
    VideoFrameOpenCV(unsigned height, unsigned width, const uint8_t * frame);
    ~VideoFrameOpenCV() {}

    unsigned getHeight() override;
    unsigned getWidth() override;

    char *getRawPointer() override;
    size_t getFrameSizeBytes() override;
    void justDisplay();
    std::shared_ptr<cv::Mat> getFrame() { return m_frame; }

    // Derived Class functions

    static std::shared_ptr<std::thread> launchDisplayThread();
private:
    std::shared_ptr<cv::Mat> m_frame = nullptr;
};


#endif /* SRC_VIDEOOPENCV_H_ */
