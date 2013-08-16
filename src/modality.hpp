#ifndef __MODALITY_HPP__
#define __MODALITY_HPP__

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
//#include <mecab.h>
#include <cabocha.h>
#include <tinyxml2.h>
#include <kcpolydb.h>

namespace linear {
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
#include <linear.h>
};


#include "sentence.hpp"

#define MODALITY_VERSION 1.0
#define LABEL_NUM 6

namespace modality {
	typedef boost::unordered_map< std::string, double > t_feat;

	enum {
		raw_text = 0,
		cabocha_text = 1,
		chapas_text = 2,
	};

	enum {
		SOURCE = 0,  // 態度表明者
		TENSE = 1,  // 時制
		ASSUMPTIONAL = 2,  // 仮想
		TYPE = 3,  // 態度
		AUTHENTICITY = 4,  // 真偽判断
		SENTIMENT = 5,  // 評価極性
	};

	typedef struct {
		int sp;
		int ep;
		std::string orthToken;
		std::string morphID;
		nlp::t_eme eme;
	} t_token;

		
	class parser {
		public:
			kyotocabinet::HashDB ttjDB;
			kyotocabinet::HashDB fadicDB;
			
//			MeCab::Tagger *mecab;
			CaboCha::Parser *cabocha;
			
			std::vector< nlp::sentence > learning_data;
			bool pred_detect_rule;
			
			kyotocabinet::HashDB l2iDB;
			kyotocabinet::HashDB f2iDB;
			boost::unordered_map< std::string, int > label2id;
			boost::unordered_map< std::string, int > feat2id;
			std::string model_path[LABEL_NUM];
			std::string feature_path[LABEL_NUM];

			linear::model *models[LABEL_NUM];
			std::vector<unsigned int> analyze_tags;
			
			parser() {
				analyze_tags.push_back(TENSE);
				analyze_tags.push_back(ASSUMPTIONAL);
				analyze_tags.push_back(TYPE);
				analyze_tags.push_back(AUTHENTICITY);
				analyze_tags.push_back(SENTIMENT);

				model_path[SOURCE] = "model_source";
				model_path[TENSE] = "model_tense";
				model_path[ASSUMPTIONAL] = "model_assumptional";
				model_path[TYPE] = "model_type";
				model_path[AUTHENTICITY] = "model_authenticity";
				model_path[SENTIMENT] = "model_sentiment";

				feature_path[SOURCE] = "feature_source";
				feature_path[TENSE] = "feature_tense";
				feature_path[ASSUMPTIONAL] = "feature_assumptional";
				feature_path[TYPE] = "feature_type";
				feature_path[AUTHENTICITY] = "feature_authenticity";
				feature_path[SENTIMENT] = "feature_sentiment";

				//				mecab = MeCab::createTagger("-p");
				if (!ttjDB.open("dic/ttjcore2seq.kch", kyotocabinet::HashDB::OREADER)) {
					std::cerr << "open error: ttjcore2seq: " << ttjDB.error().name() << std::endl;
				}
				if (!fadicDB.open("dic/FAdic.kch", kyotocabinet::HashDB::OREADER)) {
					std::cerr << "open error: fadic: " << fadicDB.error().name() << std::endl;
				}
				cabocha = CaboCha::createParser("-f1");
				
				pred_detect_rule = false;

				if (!l2iDB.open("label2id.kch", kyotocabinet::HashDB::OREADER)) {
					std::cerr << "open error: label2id: " << l2iDB.error().name() << std::endl;
				}

				if (!f2iDB.open("feat2id.kch", kyotocabinet::HashDB::OREADER)) {
					std::cerr << "open error: feat2id: " << f2iDB.error().name() << std::endl;
				}
			}

			~parser() {
				if (!ttjDB.close()) {
					std::cerr << "close error: ttjcore2seq: " << ttjDB.error().name() << std::endl;
				}
				if (!fadicDB.close()) {
					std::cerr << "close error: fadic: " << fadicDB.error().name() << std::endl;
				}
				
				if (!l2iDB.close()) {
					std::cerr << "close error: label2id: " << l2iDB.error().name() << std::endl;
				}
				if (!f2iDB.close()) {
					std::cerr << "close error: feat2id: " << f2iDB.error().name() << std::endl;
				}
			}

		public:
			std::string id2tag(unsigned int);

			bool load_models();
			bool load_models(std::string *);
			nlp::sentence analyze(std::string, int);
			nlp::sentence analyze(nlp::sentence);
			linear::feature_node* pack_feat_linear(t_feat *, bool);
//			bool parse(std::string);
			void load_xmls(std::vector< std::string >);
			void load_deppasmods(std::vector< std::string >);
			void learn(std::string *, std::string *);
			void learn();

			nlp::sentence make_tagged_ipasents( std::vector< t_token > );

			std::vector< std::vector< t_token > > parse_OC(std::string);
			std::vector< std::vector< t_token > > parse_OW_PB_PN(std::string);
			std::vector< std::vector< t_token > > parse_OC_sents (tinyxml2::XMLElement *);
			std::vector<t_token> parse_bccwj_sent(tinyxml2::XMLElement *, int *);
			void parse_modtag_for_sent(tinyxml2::XMLElement *, std::vector< t_token > *);

			void gen_feature(nlp::sentence, int, t_feat &);
			void gen_feature_follow_mod(nlp::sentence, int, t_feat &);
			void gen_feature_function(nlp::sentence, int, t_feat &);
			void gen_feature_basic(nlp::sentence, int, t_feat &, int);
			void gen_feature_follow_chunks(nlp::sentence, int, t_feat &);
			void gen_feature_ttj(nlp::sentence, int, t_feat &);
			void gen_feature_fadic(nlp::sentence, int, t_feat &);
			
			void save_hashDB();
			void load_hashDB();
	};
};

#endif

