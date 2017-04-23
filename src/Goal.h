#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
using namespace std;
using namespace cv;

class Goal
{
public:
	Goal();
	~Goal(void);

	Goal(string name);

	int getXPos() {return this->xPos;}
	void setXPos(int x) {this->xPos = x;}

	int getYPos() {return this->yPos;}
	void setYPos(int y) {this->yPos = y;}

	int getWidth() {return this->width;}
	void setWidth(int w) {this->width = w;}

	int getHeight() {return this->height;}
	void setHeight(int h) {this->height = h;}

	Scalar getHSVmin() {return this->HSVmin;}
	void setHSVmin(Scalar min) {this->HSVmin = min;}

	Scalar getHSVmax() {return this->HSVmax;}
	void setHSVmax(Scalar max) {this->HSVmax = max;}

	string getType(){return type;}
	void setType(string t){type = t;}

	Scalar getColour() {return Colour;}
	void setColour(Scalar c){Colour = c;}

private:

	int xPos, yPos;
	int width, height;
	string type;
	Scalar HSVmin, HSVmax;
	Scalar Colour;
};
