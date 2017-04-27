// Based on code from Kyle Hounslow
// https://www.youtube.com/user/khounslow

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "High Goal Vision.h"
#include "NetworkTablesClient.h"

//initial min and max HSV filter values.
int H_MIN = 0;
int H_MAX = 0;
int S_MIN = 0;
int S_MAX = 0;
int V_MIN = 0;
int V_MAX = 0;

// OpenCV camera id to open, typically 0 for single cam setups.
int cam_id = 0;

//Globals for image width and height of display windows.
int imageWidth = 640;
int imageHeight = 480;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 5 * 5;
const int MAX_OBJECT_AREA = imageWidth*imageHeight / 1.5;
bool calibrationMode = true;//used for showing debugging windows, trackbars etc.
bool mouseIsDragging;//used for showing a rectangle on screen as user clicks and drags mouse
bool mouseMove;
bool rectangleSelected;
cv::Point initialClickPoint, currentMousePoint; //keep track of initial point clicked and current position of mouse
cv::Rect rectangleROI; //this is the ROI that the user has selected
std::vector<int> H_ROI, S_ROI, V_ROI;// HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily

NetworkTablesClient ntc;

int main(int argc, char **argv) {
	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;

	std::cout << "Entering camera check" << std::endl;

	// Create the VideoCapture object and open the requested camera.
	cv::VideoCapture capture;

	// Check if the camera is opened, if build the rest of the vectors.
	if (capture.open(cam_id)) {
		std::cout << "Opened camera: " << cam_id << std::endl;
	}
	else {
		std::cout << "Unable to open camera: " << cam_id << std::endl;
		return 0;
	}

	//Matrix to store each frame of the webcam feed
	cv::Mat cameraFeed;

	//matrix storage for HSV image
	cv::Mat HSV;

	//matrix storage for binary threshold image
	cv::Mat threshold;

	//x and y values for the location of the object
	int x = 0, y = 0;

	// Mouse callback initialization


	// must create a window before setting mouse callback
	cv::namedWindow("Image");

	// set mouse callback function to be active on "Webcam Feed" window
	// we pass the handle to our "frame" matrix so that we can draw a rectangle to it
	// as the user clicks and drags the mouse
	cv::setMouseCallback("Image", clickAndDrag_Rectangle, &cameraFeed);

	// initiate mouse move and drag to false
	mouseIsDragging = false;
	mouseMove = false;
	rectangleSelected = false;

	// Loop until 'q' is pressed
	char key = ' ';
	while (key != 'q') {
		//store image to matrix
		capture.read(cameraFeed);

		//set HSV values from user selected region
		recordHSV_Values(cameraFeed, HSV);

		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed, HSV, cv::COLOR_BGR2HSV);

		//set HSV values from user selected region
		recordHSV_Values(cameraFeed, HSV);

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
			trackFilteredObject(x, y, threshold, cameraFeed);

		// Display images
		imshow("Image", cameraFeed);

		key = cv::waitKey(10);
	}
	return 0;
}

void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param){
	//std::cout << "In clickAndDrag" << std::endl;

	//only if calibration mode is true will we use the mouse to change HSV values
	if (calibrationMode == true){
		//get handle to video feed passed in as "param" and cast as Mat pointer
		cv::Mat* videoFeed = (cv::Mat*)param;

		if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false)
		{
			//keep track of initial point clicked
			initialClickPoint = cv::Point(x, y);

			//user has begun dragging the mouse
			mouseIsDragging = true;
		}
		/* user is dragging the mouse */
		if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true)
		{
			//keep track of current mouse point
			currentMousePoint = cv::Point(x, y);
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

		if (H_ROI.size()>0){
			//NOTE: min_element and max_element return iterators so we must dereference them with "*"
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			std::cout << "MIN 'H' VALUE: " << H_MIN << std::endl;
			std::cout << "MAX 'H' VALUE: " << H_MAX << std::endl;
		}
		if (S_ROI.size()>0){
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			std::cout << "MIN 'S' VALUE: " << S_MIN << std::endl;
			std::cout << "MAX 'S' VALUE: " << S_MAX << std::endl;
		}
		if (V_ROI.size()>0){
			V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
			V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			std::cout << "MIN 'V' VALUE: " << V_MIN << std::endl;
			std::cout << "MAX 'V' VALUE: " << V_MAX << std::endl;
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
void trackFilteredObject(int &x, int &y, cv::Mat threshold, cv::Mat &cameraFeed){

	cv::Mat temp;
	threshold.copyTo(temp);

	int width, height = -1;

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

				// Determine width and height of object.
				// Calculates the bounding rect of the largest area contour
				cv::Rect rect = cv::boundingRect(cv::Mat(contours[largestIndex]));
				cv::Point pt1, pt2;
				pt1.x = rect.x;
				pt1.y = rect.y;
				pt2.x = rect.x + rect.width;
				pt2.y = rect.y + rect.height;
				width = rect.width;
				height = rect.height;

				if(calibrationMode) {
					// Draws the rect in the original image and show it
					rectangle(cameraFeed, pt1, pt2, CV_RGB(255,0,0), 1);
				}

				// Send data via network tables.
				ntc.putData(llvm::StringRef("Gear Pos"), llvm::ArrayRef<double> {x,y,width,height});

				//draw largest contour
				//drawContours(cameraFeed, contours, largestIndex, Scalar(0, 255, 255), 2);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", cv::Point(0, 50), 1, 2, cv::Scalar(0, 0, 255), 2);
	}
}
