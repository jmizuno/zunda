#ifndef __SENTENCE_HPP__
#define __SENTENCE_HPP__

#include <iostream>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>


namespace nlp {
	typedef boost::unordered_map< std::string, std::string > t_eme;

	class pas {
		public:
			int arg_id;
			std::string pred_type;
			boost::unordered_map< std::string, int > arg_type;
			boost::unordered_map< std::string, int > args;
		public:
			pas() {
				arg_id = -1;
				pred_type = "null";
			}
		
			bool parse(const std::string &, pas *);
			bool is_pred();
	};
	
	class modality {
		public:
			std::vector<int> tids;
			boost::unordered_map< std::string, std::string > tag;
//			std::string source;
//			std::string tense;
//			std::string assumptional;
//			std::string type;
//			std::string authenticity;
//			std::string sentiment;
//			std::string focus;
		public:
			modality() {
				tag["source"] = "wr:筆者";
				tag["tense"] = "0";
				tag["assumptional"] = "0";
				tag["type"] = "0";
				tag["authenticity"] = "0";
				tag["sentiment"] = "0";
				tag["focus"] = "0";
			}
			~modality() {
			}
		public:
			void parse(const std::string &);
			void str(std::string &);
			bool negation();
			bool negation_strict();
	};

	class token {
		public:
			int id;
			std::string surf;
			std::string orig;
			std::string pos;
			std::string pos1;
			std::string pos2;
			std::string pos3;
			std::string type;
			std::string form;
			std::string read;
			std::string pron;

			// for Juman
			int pos_id;
			int pos1_id;
			int pos2_id;
			int pos3_id;
			int type_id;
			int form_id;
			std::string form2;
			int form2_id;

			// for UniDic
			std::string pos4;
			//int pos4_id;
			std::string cType;
			std::string cForm;
			std::string lForm;
			std::string lemma;
			std::string orth;
			//std::string pron;
			std::string orthBase;
			std::string pronBase;
			std::string goshu;
			std::string iType;
			std::string iForm;
			std::string fType;
			std::string fForm;

			std::string ne;
			nlp::pas pas;
			nlp::modality mod;
			bool has_mod;
			std::string sem_info;
			
			std::string judge_pos_juman;

			bool has_fsem;
			std::string fsem;

			bool parse_mecab(const std::string &, const int);
			bool parse_mecab_juman(const std::string &, const int);
			bool parse_mecab_unidic(const std::string &, const int);
			bool parse_juman(const std::string &, const int);
		public:
			token() {
				surf = "*";
				orig = "*";
				pos = "*";
				pos1 = "*";
				pos2 = "*";
				pos3 = "*";
				pos4 = "*";
				type = "*";
				form = "*";
				read = "*";
				pron = "*";
				ne = "O";
				has_mod = false;

				judge_pos_juman = "詞";

				has_fsem = false;
				fsem = "O";
			}
	};

	class chunk {
		public:
			int id;
			int dst;
			std::vector< int > dsts;
			std::vector< int > srcs;
			double score;
			std::string type;
			int subj;
			int func;
			std::vector< token > tokens;
			// map global token ID to local token ID
			boost::unordered_map< int, int > tok_g2l;
		public:
			chunk() {
				score = 0.0;
				subj = 0;
				func = 0;
			}
			void str(std::string &);
			void str_orig(std::string &);
			token* get_token_has_mod();
	};

	class sentence {
		public:
			std::string input_orig;
			std::string doc_id;
			std::string sent_id;
			std::vector< chunk > chunks;
			int cid_min, cid_max, tid_min, tid_max;
			// map token ID to chunk ID which the token belongs
			boost::unordered_map< int, int > t2c;
			
			enum {
				IPADic = 0,
				JumanDic = 1,
				UniDic = 2,
			};
			int ma_dic;

			enum {
				CaboCha = 0,
				KNP = 1,
				JDepP = 2,
			};
			int da_tool;

			bool has_fsem;

/*			enum {
				SynCha = 0,
				KNP = 1,
			};
			int pas_tool;
			*/

		public:
			sentence() {
				doc_id = "";
				sent_id = "";
				cid_min = 0;
				tid_min = 0;
				
				ma_dic = IPADic;
				da_tool = CaboCha;
				has_fsem = false;
//				pas_tool = SynCha;
			}
			~sentence() {
			}
			bool parse(const std::string &);
			bool parse(const std::vector< std::string > &);
			bool pp();
			chunk* get_chunk(const int);
			chunk* get_chunk_by_tokenID(const int);
			token* get_token(const int);
			chunk* get_dst_chunk(const chunk &);
			chunk* get_dst_chunk(const chunk &, const unsigned int);
			void str(std::string &, const std::string &);
			void str(std::string &);
			void cabocha(std::string &);
			void clear_mod();
		protected:
			bool parse_cabocha(const std::vector< std::string > &);
			bool parse_knp(const std::vector< std::string > &);
	};
};

#endif

