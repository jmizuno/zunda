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
#include "cdbpp.h"

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
	typedef boost::unordered_map< std::string, boost::unordered_map< std::string, double > > t_feat_cat;
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

	enum {
		DETECT_BY_POS = 0,
		DETECT_BY_PAS = 1,
		DETECT_BY_ML = 2,
		DETECT_BY_GOLD = 3,
	};

	typedef struct {
		int sp;
		int ep;
		std::string orthToken;
		std::string morphID;
		nlp::t_eme eme;
	} t_token;

	typedef struct {
		std::vector<int> tok_ids;
		std::string semrel;
	} t_match_func;
		
	class parser {
		public:
			cdbpp::cdbpp dbr_ttj;
			cdbpp::cdbpp dbr_fadic;

//			MeCab::Tagger *mecab;
			CaboCha::Parser *cabocha;
			
			std::vector< nlp::sentence > learning_data;
			unsigned int target_detection;
			
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

			std::string use_feats_str[LABEL_NUM];
			std::vector<std::string> use_feats[LABEL_NUM];
			
			parser(std::string model_dir = MODELDIR, std::string dic_dir = DICDIR) {
				analyze_tags.push_back(TENSE);
				analyze_tags.push_back(TYPE);
				analyze_tags.push_back(ASSUMPTIONAL);
				analyze_tags.push_back(AUTHENTICITY);
				analyze_tags.push_back(SENTIMENT);
				
				std::string use_feats_common_str = "func_surf,tok,chunk,func_sem";
				use_feats_str[TENSE] = use_feats_common_str;
				use_feats_str[TYPE] = use_feats_common_str + ",fadic_worth,mod_tense";
				use_feats_str[ASSUMPTIONAL] = use_feats_common_str;
				use_feats_str[AUTHENTICITY] = use_feats_common_str + ",fadic_authenticity,mod_type";
				use_feats_str[SENTIMENT] = use_feats_common_str + ",fadic_sentiment";
				
				BOOST_FOREACH (unsigned int i, analyze_tags) {
					boost::algorithm::split(use_feats[i], use_feats_str[i], boost::algorithm::is_any_of(","));
				}

				model_path = new boost::filesystem::path[LABEL_NUM];
				feat_path = new boost::filesystem::path[LABEL_NUM];
				set_model_dir(model_dir);

				//				mecab = MeCab::createTagger("-p");
				//				
				std::ifstream ifs_db;
				boost::filesystem::path dic_dir_path(dic_dir);
				boost::filesystem::path ttj_path("ttjcore2seq.cdb");
				ttj_path = dic_dir_path / ttj_path;
				ifs_db.open(ttj_path.string().c_str(), std::ios_base::binary);
				if (ifs_db.fail()) {
					std::cerr << "ERROR: Failed to open a database file \"" << ttj_path.string() << "\"" << std::endl;
					exit(-1);
				}
				dbr_ttj.open(ifs_db);
				ifs_db.close();

				boost::filesystem::path fadic_path("FAdic.cdb");
				fadic_path = dic_dir_path / fadic_path;
				ifs_db.open(fadic_path.string().c_str(), std::ios_base::binary);
				if (ifs_db.fail()) {
					std::cerr << "ERROR: Failed to open a database file \"" << fadic_path.string() << "\"" << std::endl;
					exit(-1);
				}
				dbr_fadic.open(ifs_db);
				ifs_db.close();

				cabocha = CaboCha::createParser("-f1");
				
				target_detection = DETECT_BY_POS;
				openDB();
			}

			~parser() {
				closeDB();
			}

		public:
			std::string id2tag(unsigned int);
			void set_model_dir(std::string);
			void set_model_dir(boost::filesystem::path);
			bool detect_target(nlp::token);

			bool load_models(boost::filesystem::path *);
			bool load_models();
			nlp::sentence analyze(std::string, int, bool);
			nlp::sentence analyze(nlp::sentence, bool);
			linear::feature_node* pack_feat_linear(t_feat, bool);
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

			void save_hashDB();
			void load_hashDB();
			void openDB_writable();
			void openDB();
			void closeDB();
	};


	class feature_generator {
		public:
			nlp::token tok_core;
			nlp::chunk chk_core, chk_dst;
			std::vector< nlp::chunk > chks_src;
			bool has_chk_dst;
			nlp::sentence sent;
			int tok_id;
			t_feat_cat feat_cat;
		public:
			void update(nlp::sentence sent) {
				tok_core = sent.get_token(tok_id);
				chk_core = sent.get_chunk_by_tokenID(tok_id);
				if (chk_core.dst != -1) {
					chk_dst = sent.get_chunk(chk_core.dst);
					has_chk_dst = true;
				}
				sent.get_chunks_src(&chks_src, chk_core);
			}

			feature_generator(nlp::sentence _sent, int _tok_id) {
				has_chk_dst = false;

				sent = _sent;
				tok_id = _tok_id;
				update(sent);
			}

			
			std::string compile_feat_str(std::vector<std::string>);
			t_feat compile_feat(std::vector<std::string>);
			void gen_feature_function();
			void gen_feature_mod(std::string);
			void gen_feature_basic(const int);
			void gen_feature_dst_chunks();
			void gen_feature_ttj(cdbpp::cdbpp *);
			void gen_feature_fadic(cdbpp::cdbpp *);
	};
};

#endif

