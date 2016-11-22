#ifndef __FUNCSEM_HPP__
#define __FUNCSEM_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#ifdef USE_CRFSUITE
#  include <crfsuite_api.hpp>
#endif
#include "sentence.hpp"


#ifndef FUNC_MODEL_IPA
#  define FUNC_MODEL_IPA "jfe_corpus_ver2.1_core.model"
#endif

#define FUNC_TERMS "する,ある,こと,もの,事,いう,言う,いい,できる,得る,える,なる"

namespace funcsem {
	class feature_generator {
	};


	class tagger {
		public:
#ifdef USE_CRFSUITE
			CRFSuite::Tagger crf_tagger;
#endif
			std::vector<std::string> func_terms;

		private:
			std::vector< std::vector< std::vector<std::string> > > target_pos;
			int max_num_tok_target;

		public:
			void tag(nlp::sentence &, const std::vector<int> &);
#ifdef USE_CRFSUITE
			bool load_model(const std::string &);
			bool tag_by_crf(nlp::sentence &, unsigned int, unsigned int);
#endif

		private:
			bool is_func(const nlp::token &);
			bool is_pred(const nlp::token &);


		public:
			tagger() {
				boost::algorithm::split(func_terms, FUNC_TERMS, boost::algorithm::is_any_of(","));
				std::sort(func_terms.begin(), func_terms.end());
			}

			~tagger() {
			}
	};
};

#endif


