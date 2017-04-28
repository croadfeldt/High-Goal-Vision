// Based on ZED SDK 2.0 openCV example code.
// Portions covered under StereoLabs copyright.

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <sl/Core.hpp>
#include <sl/defines.hpp>
#include <iostream>
#include "High Goal Vision.h"
#include "NetworkTablesClient.h"
#include <ctime>

//initial min and max HSV filter values.
int H_MIN = 0;
int H_MAX = 0;
int S_MIN = 0;
int S_MAX = 0;
int V_MIN = 0;
int V_MAX = 0;

//Globals for image width and height of display windows.
int imageWidth = 720;
int imageHeight = 404;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 1 * 1;
const int MAX_OBJECT_AREA = imageWidth*imageHeight / 1.5;

bool calibrationMode = false; //used for showing debugging windows, trackbars etc.

bool mouseIsDragging;//used for showing a rectangle on screen as user clicks and drags mouse
bool mouseMove;
bool rectangleSelected;
cv::Point initialClickPoint, currentMousePoint; //keep track of initial point clicked and current position of mouse
cv::Rect rectangleROI; //this is the ROI that the user has selected
std::vector<int> H_ROI, S_ROI, V_ROI;// HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily

// How many frames per second for the output image to the smartdashboard.
int sdFPS = 15;

NetworkTablesClient ntc;

typedef struct mouseOCVStruct {
	sl::Mat depth;
	cv::Size _resize;
} mouseOCV;

mouseOCV mouseStruct;

int main(int argc, char **argv) {
	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;

	// JPEG Image prep items
	std::vector<uchar> buff;//buffer for coding
	std::vector<int> param(2);
	param[0] = cv::IMWRITE_JPEG_QUALITY;
	param[1] = 80;//default(95) 0-100

	// Determine number of clock cycles between output frames.
	std::clock_t clocks_per_frame = CLOCKS_PER_SEC / sdFPS;
	std::clock_t last_clock = 0;

	// Turn on calibration mode if any argument is given.
	if (argc > 1) {
		calibrationMode = true;
	}

	// Create a ZED camera object
	sl::Camera zed;

	//matrix storage for HSV image
	cv::Mat HSV;

	//matrix storage for binary threshold image
	cv::Mat threshold;

	//x and y values for the location of the object
	int x = 0, y = 0;

	// Set configuration parameters
	sl::InitParameters init_params;
	init_params.camera_resolution = sl::RESOLUTION_HD720;
	init_params.depth_mode = sl::DEPTH_MODE_PERFORMANCE;
	init_params.coordinate_units = sl::UNIT_METER;

	// Open the camera
	sl::ERROR_CODE err = zed.open(init_params);
	if (err != sl::SUCCESS)
		return 1;

	// Set runtime parameters after opening the camera
	sl::RuntimeParameters runtime_parameters;
	runtime_parameters.sensing_mode = sl::SENSING_MODE_STANDARD; // Use STANDARD sensing mode

	// Create sl and cv Mat to get ZED left image and depth image
	// Best way of sharing sl::Mat and cv::Mat :
	// Create a sl::Mat and then construct a cv::Mat using the ptr to sl::Mat data.
	sl::Resolution image_size = zed.getResolution();
	sl::Mat image_zed(image_size, sl::MAT_TYPE_8U_C4); // Create a sl::Mat to handle Left image
	cv::Mat image_ocv(image_zed.getHeight(), image_zed.getWidth(), CV_8UC4, image_zed.getPtr<sl::uchar1>(sl::MEM_CPU)); // Create an OpenCV Mat that shares sl::Mat buffer
	sl::Mat depth_image_zed(image_size, sl::MAT_TYPE_8U_C4);
	cv::Mat depth_image_ocv(depth_image_zed.getHeight(), depth_image_zed.getWidth(), CV_8UC4, depth_image_zed.getPtr<sl::uchar1>(sl::MEM_CPU));

	// Create OpenCV images to display (lower resolution to fit the screen)
	cv::Size displaySize(imageWidth, imageHeight);
	cv::Mat image_ocv_display(displaySize, CV_8UC4);
	cv::Mat depth_image_ocv_display(displaySize, CV_8UC4);

	// Mouse callback initialization
	mouseStruct.depth.alloc(image_size, sl::MAT_TYPE_32F_C1);
	mouseStruct._resize = displaySize;

	// Give a name to OpenCV Windows
	cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);
	//cv::setMouseCallback("Depth", onMouseCallback, (void*) &mouseStruct);

	// must create a window before setting mouse callback
	cv::namedWindow("Image");

	// set mouse callback function to be active on "Webcam Feed" window
	// we pass the handle to our "frame" matrix so that we can draw a rectangle to it
	// as the user clicks and drags the mouse
	cv::setMouseCallback("Image", clickAndDrag_Rectangle, (void*) &mouseStruct);

	// initiate mouse move and drag to false
	mouseIsDragging = false;
	mouseMove = false;
	rectangleSelected = false;

	// Jetson only. Execute the calling thread on 2nd core
	sl::Camera::sticktoCPUCore(2);

	// Loop until 'q' is pressed
	char key = ' ';
	while (key != 'q') {

		// Get HSV values from smartdashboard if not in calibration mode.
		if (!calibrationMode) {
			getHSV();
		}

		// Pull Camera Settings from NetworkTables.
		// This will be pushed into a separate thread in next version.
		//        std::cout << "Reset all settings to default" << std::endl;
		//        zed.setCameraSettings(sl::CAMERA_SETTINGS_BRIGHTNESS, -1, true);
		//        zed.setCameraSettings(sl::CAMERA_SETTINGS_CONTRAST, -1, true);
		//        zed.setCameraSettings(sl::CAMERA_SETTINGS_HUE, -1, true);
		//        zed.setCameraSettings(sl::CAMERA_SETTINGS_SATURATION, -1, true);
		//        zed.setCameraSettings(sl::CAMERA_SETTINGS_GAIN, -1, true);
		zed.setCameraSettings(sl::CAMERA_SETTINGS_EXPOSURE, (int) ntc.getData("Exposure"), true);
		//        zed.setCameraSettings(sl::CAMERA_SETTINGS_WHITEBALANCE, -1, true);

		// Grab and display image and depth
		if (zed.grab(runtime_parameters) == sl::SUCCESS) {

			zed.retrieveImage(image_zed, sl::VIEW_LEFT); // Retrieve the left image
			zed.retrieveImage(depth_image_zed, sl::VIEW_DEPTH); //Retrieve the depth view (image)
			zed.retrieveMeasure(mouseStruct.depth, sl::MEASURE_DEPTH); // Retrieve the depth measure (32bits)

			//convert frame from BGR to HSV colorspace
			cvtColor(image_ocv, HSV, cv::COLOR_BGR2HSV);

			//set HSV values from user selected region
			recordHSV_Values(image_ocv, HSV);

			//filter HSV image between values and store filtered image to
			//threshold matrix
			inRange(HSV, cv::Scalar(H_MIN, S_MIN, V_MIN), cv::Scalar(H_MAX, S_MAX, V_MAX), threshold);

			//perform morphological operations on thresholded image to eliminate noise
			//and emphasize the filtered object(s)
			if (useMorphOps)
				morphOps(threshold);

			//pass in thresholded frame to our object tracking function
			//this function will return the x and y coordinates of the
			//filtered object
			if (trackObjects)
				trackFilteredObject(x, y, threshold, image_ocv, mouseStruct.depth);

			// If not in calibration mode, don't display image windows.
			if (calibrationMode) {
				// Resize and display with OpenCV
				cv::resize(image_ocv, image_ocv_display, displaySize);
				imshow("Image", image_ocv_display);
				cv::resize(depth_image_ocv, depth_image_ocv_display, displaySize);
				imshow("Depth", depth_image_ocv_display);

				key = cv::waitKey(10);
			}

			// Prep and display the image for display on the smartdashboard.
			if (clocks_per_frame > (std::clock() - last_clock)) {
				buff.clear();
				cv::imencode(".jpg", image_ocv, buff, param);
				std::stringstream ss;
				for(size_t i = 0; i < buff.size(); ++i)
				{
					ss << buff[i];
				}
				std::string s = ss.str();
				ntc.putRaw("Image", s);
				last_clock = std::clock();
			}
		}
	}

	zed.close();
	return 0;
}

void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param){
	//std::cout << "In clickAndDrag" << std::endl;

	mouseOCVStruct* data = (mouseOCVStruct*) param;
	int y_int = (y * data->depth.getHeight() / data->_resize.height);
	int x_int = (x * data->depth.getWidth() / data->_resize.width);

	//only if calibration mode is true will we use the mouse to change HSV values
	if (calibrationMode == true){
		//get handle to video feed passed in as "param" and cast as Mat pointer
		cv::Mat* videoFeed = (cv::Mat*)param;

		if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false)
		{
			//keep track of initial point clicked
			initialClickPoint = cv::Point(x_int, y_int);

			//user has begun dragging the mouse
			mouseIsDragging = true;
		}
		/* user is dragging the mouse */
		if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true)
		{
			//keep track of current mouse point
			currentMousePoint = cv::Point(x_int, y_int);
			//user has moved the mouse while clicking and dragging
			mouseMove = true;
		}
		/* user has released left button */
		if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true)
		{
			//set rectangle ROI to the rectangle that the user has selected
			rectangleROI = cv::Rect(initialClickPoint, currentMousePoint);

			//reset boolean variables
			mouseIsDragging = false;
			mouseMove = false;
			rectangleSelected = true;
		}

		if (event == CV_EVENT_RBUTTONDOWN){
			//user has clicked right mouse button
			//Reset HSV Values
			H_MIN = 0;
			S_MIN = 0;
			V_MIN = 0;
			H_MAX = 0;
			S_MAX = 0;
			V_MAX = 0;

		}
		if (event == CV_EVENT_MBUTTONDOWN){

			//user has clicked middle mouse button
			//enter code here if needed.
		}
	}

}
void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame){

	//save HSV values for ROI that user selected to a vector
	if (mouseMove == false && rectangleSelected == true){

		//clear previous vector values
		if (H_ROI.size()>0) H_ROI.clear();
		if (S_ROI.size()>0) S_ROI.clear();
		if (V_ROI.size()>0 )V_ROI.clear();
		//if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
		if (rectangleROI.width<1 || rectangleROI.height<1) std::cout << "Please drag a rectangle, not a line" << std::endl;
		else{
			for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++){
				//iterate through both x and y direction and save HSV values at each and every point
				for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++){
					//save HSV value at this point
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}
		//reset rectangleSelected so user can select another region if necessary
		rectangleSelected = false;

		//set min and max HSV values from min and max elements of each array
		bool HSVUpdate = false;
		if (H_ROI.size()>0){
			//NOTE: min_element and max_element return iterators so we must dereference them with "*"
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			std::cout << "MIN 'H' VALUE: " << H_MIN << std::endl;
			std::cout << "MAX 'H' VALUE: " << H_MAX << std::endl;

			// Update smartdashboard values.
			HSVUpdate = true;
		}
		if (S_ROI.size()>0){
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			std::cout << "MIN 'S' VALUE: " << S_MIN << std::endl;
			std::cout << "MAX 'S' VALUE: " << S_MAX << std::endl;

			// Update smartdashboard values.
			HSVUpdate = true;
		}
		if (V_ROI.size()>0){
			V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
			V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			std::cout << "MIN 'V' VALUE: " << V_MIN << std::endl;
			std::cout << "MAX 'V' VALUE: " << V_MAX << std::endl;

			// Update smartdashboard values.
			HSVUpdate = true;
		}

		if(HSVUpdate) {
			ntc.putData("HSVVals", llvm::ArrayRef<double> {H_MIN, H_MAX, S_MIN, S_MAX, V_MIN, V_MAX});
		}

	}

	if (mouseMove == true){
		//if the mouse is held down, we will draw the click and dragged rectangle to the screen
		cv::rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
	}


}

std::string intToString(int number){


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void drawObject(int x, int y, cv::Mat &frame){

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!


	//'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window)

	cv::circle(frame, cv::Point(x, y), 20, cv::Scalar(0, 255, 0), 2);
	if (y - 25>0)
		cv::line(frame, cv::Point(x, y), cv::Point(x, y - 25), cv::Scalar(0, 255, 0), 2);
	else cv::line(frame, cv::Point(x, y), cv::Point(x, 0), cv::Scalar(0, 255, 0), 2);
	if (y + 25<imageHeight)
		cv::line(frame, cv::Point(x, y), cv::Point(x, y + 25), cv::Scalar(0, 255, 0), 2);
	else cv::line(frame, cv::Point(x, y), cv::Point(x, imageHeight), cv::Scalar(0, 255, 0), 2);
	if (x - 25>0)
		cv::line(frame, cv::Point(x, y), cv::Point(x - 25, y), cv::Scalar(0, 255, 0), 2);
	else cv::line(frame, cv::Point(x, y), cv::Point(0, y), cv::Scalar(0, 255, 0), 2);
	if (x + 25<imageWidth)
		cv::line(frame, cv::Point(x, y), cv::Point(x + 25, y), cv::Scalar(0, 255, 0), 2);
	else cv::line(frame, cv::Point(x, y), cv::Point(imageWidth, y), cv::Scalar(0, 255, 0), 2);

	cv::putText(frame, intToString(x) + "," + intToString(y), cv::Point(x, y + 30), 1, 1, cv::Scalar(0, 255, 0), 2);

}
void morphOps(cv::Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8, 8));

	cv::erode(thresh, thresh, erodeElement);
	cv::erode(thresh, thresh, erodeElement);


	cv::dilate(thresh, thresh, dilateElement);
	cv::dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, cv::Mat threshold, cv::Mat &cameraFeed, sl::Mat &depth){

	cv::Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	int largestIndex = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				cv::Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we save a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
					//save index of largest contour to use with drawContours
					largestIndex = index;
				}
				else {
					std::cout << "Found item, but did not match size range." <<std::endl;
					objectFound = false;
				}
				//these will be changed using trackbars

			}

			//let user know you found an object
			if (objectFound == true){
				putText(cameraFeed, "Tracking Object", cv::Point(0, 50), 2, 1, cv::Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);

				sl::float1 dist;
				depth.getValue(x, y, &dist);

				std::cout << std::endl;
				if (isValidMeasure(dist)) {
					//					std::cout << "Depth at (" << x << "," << y << ") : " << dist << "m";
					ntc.putData(llvm::StringRef("High Goal Pos"), llvm::ArrayRef<double> {x,y,dist});
				}
				else {
					std::string depth_status;
					//					if (dist == TOO_FAR) depth_status = ("Depth is too far.");
					//					else if (dist == TOO_CLOSE) depth_status = ("Depth is too close.");
					//					else depth_status = ("Depth not available");
					//					std::cout << depth_status;
				}
				//				std::cout << std::endl;

				//draw largest contour
				//drawContours(cameraFeed, contours, largestIndex, Scalar(0, 255, 255), 2);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", cv::Point(0, 50), 1, 2, cv::Scalar(0, 0, 255), 2);
	}
}

static void onMouseCallback(int32_t event, int32_t x, int32_t y, int32_t flag, void * param) {
	if (event == CV_EVENT_LBUTTONDOWN) {
		mouseOCVStruct* data = (mouseOCVStruct*) param;
		int y_int = (y * data->depth.getHeight() / data->_resize.height);
		int x_int = (x * data->depth.getWidth() / data->_resize.width);

		sl::float1 dist;
		data->depth.getValue(x_int, y_int, &dist);

		std::cout << std::endl;
		if (isValidMeasure(dist))
			std::cout << "Depth at (" << x_int << "," << y_int << ") : " << dist << "m";
		else {
			std::string depth_status;
			if (dist == TOO_FAR) depth_status = ("Depth is too far.");
			else if (dist == TOO_CLOSE) depth_status = ("Depth is too close.");
			else depth_status = ("Depth not available");
			std::cout << depth_status;
		}
		std::cout << std::endl;
	}
}

void getHSV() {
	// Get the HSV values from the smartdashboard.
	H_MIN = (int) ntc.getData("H_MIN");
	H_MAX = (int) ntc.getData("H_MAX");
	S_MIN = (int) ntc.getData("S_MIN");
	S_MAX = (int) ntc.getData("S_MAX");
	V_MIN = (int) ntc.getData("V_MIN");
	V_MAX = (int) ntc.getData("V_MAX");

	if (H_MIN == -1) {
		H_MIN = 0;
		std::cout << "Unable to retrieve H_MIN form smartdashboard." << std::endl;
	}
	if (H_MAX == -1) {
		H_MAX = 255;
		std::cout << "Unable to retrieve H_MAX form smartdashboard." << std::endl;
	}
	if (S_MIN == -1) {
		S_MIN = 0;
		std::cout << "Unable to retrieve S_MIN form smartdashboard." << std::endl;
	}
	if (S_MAX == -1) {
		S_MAX = 255;
		std::cout << "Unable to retrieve S_MAX form smartdashboard." << std::endl;
	}
	if (V_MIN == -1) {
		V_MIN = 0;
		std::cout << "Unable to retrieve V_MIN form smartdashboard." << std::endl;
	}
	if (V_MAX == -1) {
		V_MAX = 255;
		std::cout << "Unable to retrieve V_MAX form smartdashboard." << std::endl;
	}
}
