#ifndef __FUNCSEM_HPP__
#define __FUNCSEM_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include "../config.h"

#ifdef USE_CRFSUITE
#  include <crfsuite_api.hpp>
#endif
#include "sentence.hpp"


#ifndef FUNC_MODEL_IPA
#  define FUNC_MODEL_IPA "funcsem_v2.2.2.model"
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
			void tag(nlp::sentence &);
#ifdef USE_CRFSUITE
			bool load_model(const std::string &);
			void train(const std::string &, const std::vector< nlp::sentence > &);
#endif

		private:
			void detect_target(nlp::sentence &, std::vector< std::vector< unsigned int> > &);
			bool is_func(const nlp::token &);
			bool is_pred(const nlp::token &);
#ifdef USE_CRFSUITE
			void gen_feat(nlp::sentence &, unsigned int, unsigned int, CRFSuite::ItemSequence &);
			bool tag_by_crf(nlp::sentence &, unsigned int, unsigned int);
#endif


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


