#ifndef __MODALITY_HPP__
#define __MODALITY_HPP__

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <mecab.h>
#include <cabocha.h>
#include <tinyxml2.h>
#include "sentence.hpp"

#define MODALITY_VERSION 1.0

namespace modality {
	typedef boost::unordered_map< std::string, double > t_feat;
	typedef struct {
		int sp;
		int ep;
		std::string orthToken;
		std::string morphID;
	} t_token;

	class instance {
		public:
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
			MeCab::Tagger *mecab;
			CaboCha::Parser *cabocha;
			parser() {
				mecab = MeCab::createTagger("-p");
				cabocha = CaboCha::createParser("-I1 -f1");
			}
			~parser() {
			}

		public:
			bool parse(std::string);
			bool learnOC(std::string);
			std::vector<t_token> parseOCXML_sent(tinyxml2::XMLElement *, int *);
			bool conv_instance_OC(std::string, t_feat &);
			bool gen_feature(nlp::sentence, int, t_feat &);
			bool gen_feature_basic(nlp::sentence, int, t_feat &, int);
	};
};

#endif
