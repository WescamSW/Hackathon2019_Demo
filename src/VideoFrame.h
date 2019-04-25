/*
 * VideoFrame.h
 *
 *  Created on: Apr 15, 2019
 *      Author: slascos
 */

#ifndef SRC_VIDEOFRAME_H_
#define SRC_VIDEOFRAME_H_

class VideoFrame {
public:
    VideoFrame() = delete;
    VideoFrame(unsigned height, unsigned width) {}
    virtual ~VideoFrame() = default;

    virtual unsigned getHeight()         = 0;
    virtual unsigned getWidth()          = 0;
    virtual char*    getRawPointer()     = 0;
    virtual size_t   getFrameSizeBytes() = 0;
};

#endif /* SRC_VIDEOFRAME_H_ */
