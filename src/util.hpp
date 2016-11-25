#pragma once

#include <vector>
#include <boost/filesystem.hpp>


bool mkdir(const boost::filesystem::path &);

template<typename Tins> void get_subvec(std::vector< std::vector< Tins > > *res, const std::vector< Tins > &vec, size_t ns, size_t ne) {
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

