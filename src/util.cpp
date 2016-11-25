#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <string>
#include "util.hpp"

bool mkdir(const boost::filesystem::path &dir_path) {
	std::cerr << dir_path.string() << std::endl;
	if (!boost::filesystem::exists(dir_path)) {
		std::cerr << "mkdir " << dir_path.string() << std::endl;
		if (!boost::filesystem::create_directories(dir_path)) {
			std::cerr << "ERROR: mkdir " << dir_path.string() << " failed" << std::endl;
			return false;
		}
	}
	return true;
}


