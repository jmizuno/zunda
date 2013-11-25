#ifndef __SENTENCE_HPP__
#define __SENTENCE_HPP__

#include <iostream>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>


std::string join(std::vector<std::string>, std::string);

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
		
			bool parse(std::string, pas *);
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
			void parse(std::string);
			std::string str();
			bool negation();
			bool negation_strict();
	};

	class token {
		public:
			int id;
			std::string surf;
			std::string orig;
			std::string pos;
			int pos_id;
			std::string pos1;
			int pos1_id;
			std::string pos2;
			int pos2_id;
			std::string pos3;
			int pos3_id;
			std::string type;
			int type_id;
			std::string form;
			int form_id;
			std::string form2;
			int form2_id;
			std::string read;
			std::string pron;
			std::string ne;
			nlp::pas pas;
			nlp::modality mod;
			bool has_mod;
			std::string sem_info;
			
			boost::regex juman_pos;

			bool parse_chasen(std::string, int);
			bool parse_mecab(std::string, int);
			bool parse_juman(std::string, int);
		public:
			token() {
				surf = "*";
				orig = "*";
				pos = "*";
				pos1 = "*";
				pos2 = "*";
				pos3 = "*";
				type = "*";
				form = "*";
				read = "*";
				pron = "*";
				ne = "O";
				has_mod = false;

				juman_pos = ".*詞";
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
			bool has_mod;
		public:
			chunk() {
				score = 0.0;
				has_mod = false;
				subj = 0;
				func = 0;
			}
			bool add_token(token);
			std::string str();
			std::string str_orig();
			token get_token_has_mod();
	};

	class sentence {
		public:
			std::string input_orig;
			std::string doc_id;
			std::string sent_id;
			std::vector< chunk > chunks;
			int cid_min, cid_max, tid_min, tid_max;
			boost::unordered_map< int, int > t2c;
			
			enum{
				MeCab = 0,
				ChaSen = 1,
			};
			int ma_tool;
			
			enum {
				IPADic = 0,
				UniDic = 1,
			};
			int ma_dic;
			
		private:
//			boost::regex chk_line("^\\* [0-9][0-9]* ");
//			boost::regex chk_dst("^([0-9\\-]+)[A-Z]$");
//			std::string dst_rep = "$1";
			boost::regex chk_line;
			boost::regex chk_dst;
			std::string dst_rep;
			std::string type_rep;
			boost::regex chk_line_knp;
			boost::regex chk_basic_knp;
		
		public:
			sentence() {
				doc_id = "";
				sent_id = "";
				cid_min = 0;
				tid_min = 0;
				
				chk_line = "^\\* [0-9][0-9]* ";
				chk_dst = "^([0-9\\-]+)([A-Z])$";
				dst_rep = "$1";
				type_rep = "$2";
				chk_line_knp = "^\\* [0-9-]+[A-Z] ";
				chk_basic_knp = "^\\+ [0-9-]+[A-Z] ";
				
				ma_tool = ChaSen;
				ma_dic = IPADic;
			}
			bool test(std::vector< std::string >);
			bool add_chunk(chunk);
			bool parse_cabocha(std::string);
			bool parse_cabocha(std::vector< std::string >);
			bool parse_knp(std::string);
			bool parse_knp(std::vector< std::string >);
			bool pp();
			chunk get_chunk(int);
			chunk get_chunk_by_tokenID(int);
			bool get_dep_chunk(chunk*, chunk);
			bool get_dep_chunk(chunk*, int);
			bool get_dep_chunk(chunk*, chunk, int);
			bool get_dep_chunk(chunk*, int, int);
			bool get_chunks_src(std::vector< nlp::chunk > *, nlp::chunk);
			bool get_chunks_src(std::vector< nlp::chunk > *, int);
			token get_token(int);
			bool get_chunk_has_mod(chunk&, int);
			std::string str(std::string);
			std::string str();
			std::string cabocha();
			void clear_mod();
	};
};

#endif

