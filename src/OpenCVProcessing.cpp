/*
 * Bebop2Demo.cpp
 *
 *  Created on: Feb 1, 2019
 *      Author: slascos
 */
#include "OpenCVProcessing.h"

using namespace std;
using namespace cv;

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
                circle (cornersNormScaled, Point(i,j), 5, Scalar(0), 2, 8, 0);
            }
        }
    }
    outputImage = cornersNormScaled;
}

void openCVProcessing(shared_ptr<Mat> imageToProcess, bool *processingDone)
{
    // Convert to grayscale
    Mat grayImage;
    cvtColor(*imageToProcess, grayImage, COLOR_BGR2GRAY);

    //harrisCorner(grayImage, grayImage);

    //*imageToProcess = grayImage;
    *processingDone = true;
}
