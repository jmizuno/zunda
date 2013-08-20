#ifndef __MODALITY_HPP__
#define __MODALITY_HPP__

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
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

#ifndef MODELDIR
#define MODELDIR "model"
#endif
#ifndef DICDIR
#define DICDIR "dic"
#endif


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
			boost::filesystem::path *model_path;
			boost::filesystem::path *feat_path;
			boost::filesystem::path f2i_path;
			boost::filesystem::path l2i_path;

			linear::model *models[LABEL_NUM];
			std::vector<unsigned int> analyze_tags;
			
			parser(std::string model_dir = MODELDIR, std::string dic_dir = DICDIR) {
				analyze_tags.push_back(TENSE);
				analyze_tags.push_back(ASSUMPTIONAL);
				analyze_tags.push_back(TYPE);
				analyze_tags.push_back(AUTHENTICITY);
				analyze_tags.push_back(SENTIMENT);
				
				model_path = new boost::filesystem::path[LABEL_NUM];
				feat_path = new boost::filesystem::path[LABEL_NUM];
				set_model_dir(model_dir);

				//				mecab = MeCab::createTagger("-p");
				//				
				boost::filesystem::path dic_dir_path(dic_dir);
				boost::filesystem::path ttj_path("ttjcore2seq.kch");
				ttj_path = dic_dir_path / ttj_path;
				if (!ttjDB.open(ttj_path.string().c_str(), kyotocabinet::HashDB::OREADER)) {
					std::cerr << "open error: ttjcore2seq: " << ttjDB.error().name() << std::endl;
					exit(-1);
				}
				boost::filesystem::path fadic_path("FAdic.kch");
				fadic_path = dic_dir_path / fadic_path;
				if (!fadicDB.open(fadic_path.string().c_str(), kyotocabinet::HashDB::OREADER)) {
					std::cerr << "open error: fadic: " << fadicDB.error().name() << std::endl;
					exit(-1);
				}
				cabocha = CaboCha::createParser("-f1");
				
				pred_detect_rule = false;
				openDB();
			}

			~parser() {
				if (!ttjDB.close()) {
					std::cerr << "close error: ttjcore2seq: " << ttjDB.error().name() << std::endl;
				}
				if (!fadicDB.close()) {
					std::cerr << "close error: fadic: " << fadicDB.error().name() << std::endl;
				}
				
				closeDB();
			}

		public:
			std::string id2tag(unsigned int);
			void set_model_dir(std::string);
			void set_model_dir(boost::filesystem::path);

			bool load_models(boost::filesystem::path *);
			bool load_models();
			nlp::sentence analyze(std::string, int);
			nlp::sentence analyze(nlp::sentence);
			linear::feature_node* pack_feat_linear(t_feat *, bool);
//			bool parse(std::string);
			void load_xmls(std::vector< std::string >);
			void load_deppasmods(std::vector< std::string >);
			void learn(boost::filesystem::path *, boost::filesystem::path *);
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
			void openDB_writable();
			void openDB();
			void closeDB();
	};
};

#endif

