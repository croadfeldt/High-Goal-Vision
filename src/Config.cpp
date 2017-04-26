/*
 * Config.cpp
 *
 *  Created on: Apr 25, 2017
 *      Author: ubuntu
 */

#include "Config.h"
#include "yaml-cpp/yaml.h"
#include <boost/filesystem.hpp>

Config::Config() {
}

Config::Config(std::string config_file_path) : config_file_path(config_file_path) {
	this->readConfig();
}

Config::~Config() {
	// TODO Auto-generated destructor stub
}

int Config::readConfig() {
	// Spin up a YAML object, if the config file exists.
	if (boost::filesystem::exists(config_file_path)) {
		YAML::Node config = YAML::LoadFile(config_file_path);
		std::cout << YAML::Dump(config) << std::endl;

		// Parse the config file, for now just read in the HSV ranges.
		const YAML::Node hsv_ranges = config["hsv_ranges"];
		std::cout << YAML::Dump(hsv_ranges) << std::endl;
		for (YAML::const_iterator it = hsv_ranges.begin(); it != hsv_ranges.end(); ++it) {
			std::cout << YAML::Dump(*it) << std::endl;
			HSVFilter hsv_filter;
			hsv_filter.name = (*it)["name"].as<std::string>();
			hsv_filter.hsv_range = (*it)["hsv_range"].as<std::vector<int>>();
			hsv_filters.push_back(hsv_filter);
		}
	}
	else {
		std::cout << "No config file found." << std::endl;
	}
}

int Config::readConfig(std::string config_file_path) {
	// Reuse existing readConfig function.
	this->config_file_path = config_file_path;
	return(this->readConfig());
}


