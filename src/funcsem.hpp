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

#if defined(USE_CRFSUITE)
#  include <crfsuite_api.hpp>
#elif defined(USE_CRFPP)
#  include <crfpp.h>
#endif
#include "sentence.hpp"


#ifndef FUNC_MODEL_IPA
#  if defined(USE_CRFSUITE)
#    define FUNC_MODEL_IPA "funcsem_v2.2.2.model"
#  elif defined(USE_CRFPP)
#    define FUNC_MODEL_IPA "funcsem_v2.2.2.crfpp.model"
#  endif
#endif

#define FUNC_TERMS "こと,もの,事"

namespace funcsem {
	class feature_generator {
	};


	class tagger {
		public:
			boost::filesystem::path model_path;
		private:
#if defined(USE_CRFSUITE)
			CRFSuite::Tagger fsem_tagger_crfs;
#elif defined(USE_CRFPP)
			crfpp_model_t *fsem_tagger_crfpp_model;
			crfpp_t *fsem_tagger_crfpp;
#endif
			std::vector<std::string> func_terms;

		private:
			std::vector< std::vector< std::vector<std::string> > > target_pos;
			int max_num_tok_target;

		public:
			void set_model_dir(const std::string &);
			void set_model_dir(const boost::filesystem::path &);
			bool load_model(const std::string &);
			bool load_model(const boost::filesystem::path &);
			bool load_model();
			void tag(nlp::sentence &);
#if defined(USE_CRFSUITE)
			void train_crfsuite(const std::string &, const std::vector< nlp::sentence > &);
			void print_labels_crfsuite();
#endif

		private:
			void detect_target_gold(const nlp::sentence &, std::vector< std::vector< unsigned int > > &);
			void detect_target(const nlp::sentence &, std::vector< std::vector< unsigned int> > &);
			bool is_func(const nlp::token &);
			bool is_pred(const nlp::token &);
#if defined(USE_CRFSUITE)
			bool load_model_crfsuite();
			void gen_feat_crfsuite(nlp::sentence &, unsigned int, unsigned int, CRFSuite::ItemSequence &);
			bool tag_by_crfsuite(nlp::sentence &, unsigned int, unsigned int);
#elif defined(USE_CRFPP)
			bool load_model_crfpp();
			void tag_by_crfpp(nlp::sentence &, unsigned int, unsigned int );
#endif


		public:
			tagger() {
				boost::algorithm::split(func_terms, FUNC_TERMS, boost::algorithm::is_any_of(","));
				std::sort(func_terms.begin(), func_terms.end());
			}

			~tagger() {
#if defined(USE_CRFPP)
				crfpp_model_destroy(fsem_tagger_crfpp_model);
#endif
			}
	};
};

#endif


