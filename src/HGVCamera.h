/*
 * HGVCamera.h
 *
 *  Created on: Apr 25, 2017
 *      Author: ubuntu
 */

#ifndef HGVHGVCamera_H_
#define HGVHGVCamera_H_
#include <opencv2/opencv.hpp>

class HGVCamera {
public:
	HGVCamera();
	virtual ~HGVCamera();
	int openHGVCamera();
	int openHGVCamera(int id, std::string type);
private:
	std::string name;
	std::string type;
	int id;
	int width;
	int height;
	std::string depth_mode;
	std::string units;
	cv::Ptr<cv::VideoCapture> capture;
};

#endif /* HGVHGVCamera_H_ */
