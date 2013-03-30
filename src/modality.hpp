#ifndef __MODALITY_HPP__
#define __MODALITY_HPP__

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include "sentence.hpp"

#define MODALITY_VERSION 1.0

namespace modality {
	class instance {
		public:
			typedef boost::unordered_map< std::string, double > t_feat;
			std::string label;
			t_feat feat;

		public:
			instance() {
			}
			~instance() {
			}
	};

	class parser {
		public:
			parser() {
			}
			~parser() {
			}

		public:
			bool parse(std::string);
			bool learnOC(std::string);
			bool conv_instance_OC(std::string, instance::t_feat &);
			bool gen_feature(nlp::sentence, int, instance::t_feat &);
			bool gen_feature_basic(nlp::sentence, int, instance::t_feat &, int);
	};
};

#endif
