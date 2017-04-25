/*
 * Camera.h
 *
 *  Created on: Apr 25, 2017
 *      Author: ubuntu
 */

#ifndef CAMERA_H_
#define CAMERA_H_

class Camera {
public:
	Camera();
	virtual ~Camera();
private:
	std::string name;
	std::string type;
	int id;
	int width;
	int height;
	std::string depth_mode;
	std::string units;
};

#endif /* CAMERA_H_ */
