#ifndef __MODALITY_HPP__
#define __MODALITY_HPP__

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <mecab.h>
#include <cabocha.h>
#include <tinyxml2.h>
#include <linear.h>
#include <classias/classias.h>
#include <classias/classify/linear/multi.h>
#include <classias/data.h>
#include <classias/train/lbfgs.h>
#include <classias/train/online_scheduler.h>
#include <classias/feature_generator.h>

#include "sentence.hpp"

#define MODALITY_VERSION 1.0

namespace modality {
	typedef boost::unordered_map< std::string, double > t_feat;

	typedef classias::train::lbfgs_logistic_multi<classias::msdata> trainer_type;

/*	typedef classias::train::online_scheduler_multi<
		classias::msdata,
		classias::train::averaged_perceptron_multi<
			classias::classify::linear_multi<classias::weight_vector>
		>
	> trainer_type;
	*/

	typedef struct {
		int sp;
		int ep;
		std::string orthToken;
		std::string morphID;
		nlp::t_eme eme;
	} t_token;

	class instance {
		public:
			std::string label;
			t_feat *feat;

		public:
			instance() {
				feat = new t_feat;
			}
			~instance() {
			}
	};

		
	class parser {
		public:
//			MeCab::Tagger *mecab;
			CaboCha::Parser *cabocha;
			parser() {
//				mecab = MeCab::createTagger("-p");
				cabocha = CaboCha::createParser("-f1");
			}
			~parser() {
			}

		public:
//			bool parse(std::string);
			bool learnOC(std::vector< std::string >, std::string, std::string);

			nlp::sentence make_tagged_ipasents( std::vector< t_token > );

			std::vector< std::vector< t_token > > parse_OC(std::string);
			std::vector< std::vector< t_token > > parse_OC_sents (tinyxml2::XMLElement *);
			std::vector<t_token> parse_OC_sent(tinyxml2::XMLElement *, int *);
			bool parse_OC_modtag(tinyxml2::XMLElement *, std::vector< std::vector< t_token > > *);

			bool gen_feature(nlp::sentence, int, t_feat &);
			bool gen_feature_follow_mod(nlp::sentence, int, t_feat &);
			bool gen_feature_function(nlp::sentence, int, t_feat &);
			bool gen_feature_basic(nlp::sentence, int, t_feat &, int);
	};
};

#endif
