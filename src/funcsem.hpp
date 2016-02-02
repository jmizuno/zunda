#ifndef __FUNCSEM_HPP__
#define __FUNCSEM_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <crfsuite_api.hpp>
#include "sentence.hpp"


#ifndef FUNC_MODEL_IPA
#  define FUNC_MODEL_IPA "1627.m"
#endif

#define FUNC_TERMS "する,ある,こと,もの,事,いう,言う,いい,できる,得る,える,なる"

namespace funcsem {
	class feature_generator {
	};


	class tagger {
		public:
			CRFSuite::Tagger crf_tagger;
			std::vector<std::string> func_terms;

		public:
			void tag(nlp::sentence &);
			bool tag_by_crf(std::vector<nlp::token *> &);

		public:
			tagger(const std::string &model_dir) {
				boost::filesystem::path model_path(FUNC_MODEL_IPA);
				boost::filesystem::path model_dir_path(model_dir);
				model_path = model_dir_path / model_path;

				if (crf_tagger.open(model_path.string())) {
#ifdef _MODEBUG
					std::cerr << "opened " << model_path.string() << std::endl;
#endif
				}
				else {
					std::cerr << "error: opening " << model_path.string() << std::endl;
					exit(-1);
				}


				boost::algorithm::split(func_terms, FUNC_TERMS, boost::algorithm::is_any_of(","));
				std::sort(func_terms.begin(), func_terms.end());
			}

			~tagger() {
			}
	};
};

#endif


