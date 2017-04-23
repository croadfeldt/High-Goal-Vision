#include "Goal.h"



Goal::Goal()
{
	//set values for default constructor
	setType("null");
	setColour(Scalar(0,0,0));
}

Goal::Goal(string name){

	setType(name);
	
	if(name=="high_goal"){
		// Set some defaults values, these will be overwritten by saved values.
		this->setHSVmin(Scalar(0,0,0));
		this->setHSVmax(Scalar(255,255,255));

		//BGR value for Green:
		this->setColour(Scalar(0,255,0));

		this->setWidth(15);
		this->setHeight(5);
	}

	if(name=="gear_peg"){

		this->setHSVmin(Scalar(0,0,0));
		this->setHSVmax(Scalar(255,255,255));

		//BGR value for Yellow:
		this->setColour(Scalar(0,255,255));

		this->setWidth(5);
		this->setHeight(15);
	}
}

Goal::~Goal(void)
{
}
