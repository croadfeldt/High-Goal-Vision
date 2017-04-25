/*
 * High Goal Vision.h
 *
 *  Created on: Apr 20, 2017
 *      Author: ubuntu
 */

#ifndef HIGH_GOAL_VISION_H_
#define HIGH_GOAL_VISION_H_

#include <sstream>
#include <string>
#include <opencv2/core.hpp>

std::string config_file_name = "high_goal_config.yaml";
static void onMouseCallback(int32_t event, int32_t x, int32_t y, int32_t flag, void * param);
void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param);
void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame);
std::string intToString(int number);
void drawObject(int x, int y, cv::Mat &frame);
void morphOps(cv::Mat &thresh);
void trackFilteredObject(int &x, int &y, cv::Mat threshold, cv::Mat &cameraFeed, sl::Mat &depth);
static void onMouseCallback(int32_t event, int32_t x, int32_t y, int32_t flag, void * param);

#endif /* HIGH_GOAL_VISION_H_ */
