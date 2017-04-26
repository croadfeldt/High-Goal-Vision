/*
 * Config.h
 *
 *  Created on: Apr 25, 2017
 *      Author: ubuntu
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>

class Config {
public:
	Config();
	Config(std::string config_file_path);
	virtual ~Config();
	int readConfig();
	int readConfig(std::string config_file_path);
private:
	std::string config_file_path = "high_goal_config.yaml";
};

#endif /* CONFIG_H_ */
