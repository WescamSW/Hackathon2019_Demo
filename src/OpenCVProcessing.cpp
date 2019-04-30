/*
 * Bebop2Demo.cpp
 *
 *  Created on: Feb 1, 2019
 *      Author: slascos
 */
#include "OpenCVProcessing.h"

using namespace std;
using namespace cv;

#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
//#include "opencv2/core/cuda.hpp"
//#include "opencv2/cudaimgproc.hpp"

using namespace cv;
using namespace cv::dnn;
using namespace cv::xfeatures2d;

void harrisCorner(Mat &grayImage, Mat &outputImage)
{
    Mat corners, cornersNorm, cornersNormScaled;
    int thresh = 100;

    cornerHarris(grayImage, corners, 7, 5, 0.05, BORDER_DEFAULT);
    normalize(corners, cornersNorm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    convertScaleAbs(cornersNorm, cornersNormScaled);

    // Draw circles around corners
    for (int j = 0; j < cornersNorm.rows; j++) {
        for (int i = 0; i < cornersNorm.cols; i++) {
            if ( (int) cornersNorm.at<float>(j,i) > thresh ) {
                circle (cornersNormScaled, Point(i,j), 5, Scalar(255), 2, 8, 0);
            }
        }
    }
    cornersNormScaled.copyTo(outputImage);
}


void openCVProcessing(shared_ptr<Mat> imageToProcess, bool *processingDone)
{
    // Convert to grayscale
    Mat grayImage;
    cv::cvtColor(*imageToProcess, grayImage, COLOR_BGR2GRAY);

    // Show the harris corners in the grayscale image
    Mat outputImage;
    harrisCorner(grayImage, outputImage);
    outputImage.copyTo(*imageToProcess);

    *processingDone = true;
}
