#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <boost/filesystem.hpp>


static bool mkdir(const boost::filesystem::path &dir_path) {
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


template< typename Tattr, typename Tspl >
static void join( std::string &str, const std::vector< Tattr > &vec, const Tspl &splitter ) {
	std::stringstream ss;
	if (vec.size() != 0) {
		ss << vec[0];
		if (vec.size() != 1) {
			for (unsigned int i=1 ; i<vec.size() ; ++i) {
				ss << splitter << vec[i];
			}
		}
	}
	str = ss.str();
}


template< typename Tins>
static void get_subvec(std::vector< std::vector< Tins > > *res, const std::vector< Tins > &vec, size_t ns, size_t ne) {
	size_t len_vec = vec.size();
	if (ns <= 0 || ns > len_vec)
		ns = 1;
	if (ne <= 0 || ne > len_vec)
		ne = len_vec;

	for (size_t n=ne ; n>=ns ; --n) {
		for (size_t i=0 ; i<=len_vec-n ; ++i) {
			std::vector< Tins > sub_vec;
			for (size_t j=i ; j<i+n ; ++j) {
				sub_vec.push_back(vec[j]);
			}
			res->push_back(sub_vec);
		}
	}
}


