/*
 * VideoOpenCV.cpp
 *
 *  Created on: Apr 15, 2019
 *      Author: Reg Marr
 *
 *  Description: This module is used to handle requests for image data
 *  using opencv calls. It also provides a method for displaying video
 *  and images using opencv (launchDisplayThread)
 */
#include <vector>
#include <iostream>

#include "VideoFrameOpenCV.h"

using namespace std;
using namespace cv;

VideoFrameOpenCV::VideoFrameOpenCV(unsigned height, unsigned width)
: VideoFrame(height, width)
{
    m_frame = std::make_shared<cv::Mat>(height,width,CV_8UC3);
}

VideoFrameOpenCV::VideoFrameOpenCV(unsigned height, unsigned width, const uint8_t * frame)
: VideoFrame(height, width)
{
    m_frame = make_shared<Mat>(height, width, CV_8UC3, (void * )frame);
}

char *VideoFrameOpenCV::getRawPointer()
{
    return reinterpret_cast<char *>(m_frame->ptr());
}
size_t VideoFrameOpenCV::getFrameSizeBytes()
{
    return m_frame->total() * m_frame->elemSize();
}

unsigned VideoFrameOpenCV::getHeight() {
    unsigned height = 0;
    if (m_frame) {
        height =  m_frame->rows;
    }
    return height;
}

unsigned VideoFrameOpenCV::getWidth() {
    unsigned width = 0;
    if (m_frame) {
        width = m_frame->cols;
    }
    return width;
}


