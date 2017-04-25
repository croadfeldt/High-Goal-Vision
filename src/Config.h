/*
 * Config.h
 *
 *  Created on: Apr 25, 2017
 *      Author: ubuntu
 */

#ifndef CONFIG_H_
#define CONFIG_H_

class Config {
public:
	Config();
	virtual ~Config();
private:
	std::string config_file_name = "high_goal_config.yaml";
};

#endif /* CONFIG_H_ */
