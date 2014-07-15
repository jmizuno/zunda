#ifndef __MODALITY_HPP__
#define __MODALITY_HPP__

#include <iostream>
#include <boost/version.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
//#include <mecab.h>
#include <cabocha.h>
#include "../tinyxml2/tinyxml2.h"

namespace linear {
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
#include "../liblinear-1.8/linear.h"
};

#include "sentence.hpp"
#include "cdbmap.hpp"
#include "../config.h"

#ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "beta"
#endif
#define LABEL_NUM 6

#ifndef MODELDIR_IPA
#  define MODELDIR_IPA "model_ipa"
#endif
#ifndef MODELDIR_JUMAN
#  define MODELDIR_JUMAN "model_juman"
#endif

#ifndef DICDIR
#  define DICDIR "dic"
#endif

const boost::filesystem::path TMP_DIR("/tmp");

namespace modality {
	typedef boost::unordered_map< std::string, boost::unordered_map< std::string, double > > t_feat_cat;
	typedef boost::unordered_map< std::string, double > t_feat;

	enum {
		IN_RAW = 0,
		IN_DEP_CAB = 1,
		IN_DEP_KNP = 2,
		IN_PAS_SYN = 3,
		IN_PAS_KNP = 4,
		IN_XML_CAB = 5,
		IN_XML_KNP = 6
	};

	enum {
		POS_IPA = 0,
		POS_JUMAN = 1,
		POS_UNI = 2
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
			
//			boost::unordered_map< std::string, int > label2id;
//			boost::unordered_map< std::string, int > feat2id;
			boost::filesystem::path *model_path;
			boost::filesystem::path *feat_path;

			CdbMap<std::string, int> l2i;
			boost::filesystem::path l2i_path;
			boost::filesystem::path l2id_path;

			CdbMap<int, std::string> i2l;
			boost::filesystem::path i2l_path;
			boost::filesystem::path i2ld_path;

			CdbMap<std::string, int> f2i;
			boost::filesystem::path f2i_path;
			boost::filesystem::path f2id_path;

			linear::model *models[LABEL_NUM];
			bool model_loaded;
			std::vector<unsigned int> analyze_tags;

			std::string use_feats_str[LABEL_NUM];
			std::vector<std::string> use_feats[LABEL_NUM];
			
			int pos_tag;
			std::vector<std::string> target_pos;
			std::string test_pos;
			
			parser(std::string model_dir = MODELDIR_IPA, std::string dic_dir = DICDIR) {
				analyze_tags.push_back(TENSE);
				analyze_tags.push_back(TYPE);
				analyze_tags.push_back(ASSUMPTIONAL);
				analyze_tags.push_back(AUTHENTICITY);
				analyze_tags.push_back(SENTIMENT);
				
				std::string use_feats_common_str = "func_surf,tok,chunk,func_sem";
				use_feats_str[TENSE] = use_feats_common_str + ",mod_type";
				use_feats_str[TYPE] = use_feats_common_str + ",fadic_worth";
				use_feats_str[ASSUMPTIONAL] = use_feats_common_str + ",mod_type";
				use_feats_str[AUTHENTICITY] = use_feats_common_str + ",fadic_authenticity,mod_type";
				use_feats_str[SENTIMENT] = use_feats_common_str + ",fadic_sentiment,mod_type";

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
				pos_tag = POS_IPA;
				set_pos_tag(pos_tag);
				
				open_f2i_cdb();
				open_l2i_cdb();

				model_loaded = false;
			}

			~parser() {
//				delete [] model_path;
//				delete [] feat_path;
				if (model_loaded) {
					BOOST_FOREACH (unsigned int i, analyze_tags) {
//						linear::free_and_destroy_model(&models[i]);
					}
				}
			}

		public:
			std::string id2tag(unsigned int);
			void set_model_dir(std::string);
			void set_model_dir(boost::filesystem::path);
			unsigned int detect_format(std::string);
			unsigned int detect_format(std::vector<std::string>);
			bool detect_target(nlp::token &);
			void set_pos_tag(int _pos_tag) {
				pos_tag = _pos_tag;
				switch (pos_tag) {
					case POS_IPA:
						target_pos.clear();
						target_pos.push_back("動詞\t自立");
						target_pos.push_back("形容詞\t自立");
						target_pos.push_back("名詞\tサ変接続");
						target_pos.push_back("名詞\t形容動詞語幹");
						break;
					case POS_JUMAN:
						target_pos.clear();
						target_pos.push_back("動詞\t*");
						target_pos.push_back("形容詞\t*");
						target_pos.push_back("名詞\tサ変名詞");
						break;
					case POS_UNI:
						target_pos.clear();
						break;
					default:
						std::cerr << "ERROR: invalid pos tag" << std::endl;
						break;
				}
			}

			bool load_models(boost::filesystem::path *);
			bool load_models();
			bool analyze(const std::string &, const int, nlp::sentence &);
			bool analyze(nlp::sentence &);
			bool analyzeToString(nlp::sentence &, std::string &);
			bool analyzeToString(const std::string &, const int, std::string &);
			void pack_feat_linear(t_feat &, linear::feature_node *);
//			bool parse(std::string);
			void load_xmls(std::vector< std::string >, int);
			void load_deppasmods(std::vector< std::string >, int);
			void learn(boost::filesystem::path *, boost::filesystem::path *);
			void learn();

			nlp::sentence make_tagged_ipasents( std::vector< t_token >, int );
			bool make_tagged_ipasents( std::vector< t_token >, int, nlp::sentence & );

			std::vector< std::vector< t_token > > parse_OC(std::string);
			std::vector< std::vector< t_token > > parse_OW_PB_PN(std::string);
			std::vector< std::vector< t_token > > parse_OC_sents (tinyxml2::XMLElement *);
			std::vector<t_token> parse_bccwj_sent(tinyxml2::XMLElement *, int *);
			void parse_modtag_for_sent(tinyxml2::XMLElement *, std::vector< t_token > *);

			void open_f2i_cdb();
			void open_l2i_cdb();
			void open_i2l_cdb();
			void save_f2i();
			void save_l2i();
			void save_i2l();
	};

	class feature_generator2 {
		public:
			nlp::token *tok_core;
			nlp::chunk *chk_core;
			nlp::sentence *sent;
			t_feat_cat feat_cat;
		public:
			feature_generator2(nlp::sentence *_sent, nlp::chunk *chk, nlp::token *tok) {
				sent = _sent;
				tok_core = tok;
				chk_core = chk;
			}
		public:
			bool compile_feat_str( const std::vector<std::string> &, std::string & );
			bool compile_feat( const std::vector<std::string> &, t_feat & );
			void gen_feature_function();
			void gen_feature_mod(const std::string &);
			void gen_feature_basic(const int);
			void gen_feature_dst_chunks();
			void gen_feature_ttj(cdbpp::cdbpp *);
			void gen_feature_fadic(cdbpp::cdbpp *);
			/*
			void gen_feature_last_pred();
			void gen_feature_dst_chunks(const unsigned int);
			*/
	};

	/*
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
//				tok_core = sent.get_token(tok_id);
//				chk_core = sent.get_chunk_by_tokenID(tok_id);
				if (chk_core.dst != -1) {
//					chk_dst = sent.get_chunk(chk_core.dst);
					has_chk_dst = true;
				}
//				sent.get_chunks_src(&chks_src, chk_core);
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
	*/
};

#endif

