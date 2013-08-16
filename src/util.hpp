#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <string>
#include <sstream>

template< typename Tattr, typename Tspl >
static void join( std::string &str, std::vector< Tattr > vec, Tspl splitter ) {
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


#endif

