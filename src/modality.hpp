#ifndef __MODALITY_HPP__
#define __MODALITY_HPP__

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include "sentence.hpp"

#define MODALITY_VERSION 1.0

namespace modality {
	class parser {
		public:
			typedef boost::unordered_map< std::string, double > t_feat;
			
		public:
			parser() {
			}
			~parser() {
			}
			
		public:
			bool parse(std::string);
			bool gen_feature(nlp::sentence, int, t_feat &);
			bool gen_feature_basic(nlp::sentence, int, t_feat &, int);
	};
};

#endif
