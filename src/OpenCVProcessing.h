/*
 * OpenCVProcessing.h
 *
 *  Created on: Apr 15, 2019
 *      Author: slascos
 */
#include <memory>

#ifndef SRC_OPENCVPROCESSING_H_
#define SRC_OPENCVPROCESSING_H_

#include "opencv2/opencv.hpp"

void openCVProcessing(std::shared_ptr<cv::Mat> imageToProcessPtr, bool *processingDone);

#endif /* SRC_OPENCVPROCESSING_H_ */
