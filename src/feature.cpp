#include <cstdlib>
#include <iostream>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <tinyxml2.h>
#include <mecab.h>
#include <cabocha.h>

#include "sentence.hpp"
#include "modality.hpp"


namespace modality {

	void parser::gen_feature_common(nlp::sentence sent, int tok_id, t_feat &feat) {
		gen_feature_basic(sent, tok_id, feat, 3);
		gen_feature_function(sent, tok_id, feat);
		gen_feature_follow_chunks(sent, tok_id, feat);
		gen_feature_ttj(sent, tok_id, feat);
	}
		
	void parser::gen_feature_ex(nlp::sentence sent, int tok_id, t_feat &feat, const int tag) {
		switch (tag) {
			case TENSE:
				break;
			case ASSUMPTIONAL:
				break;
			case TYPE:
				break;
			case AUTHENTICITY:
				gen_feature_fadic(sent, tok_id, feat);
				break;
			case SENTIMENT:
				break;
		};
	}


	void parser::gen_feature_follow_mod(nlp::sentence sent, int tok_id, t_feat &feat) {
		nlp::chunk chk;
		chk = sent.get_chunk_by_tokenID(tok_id);
		
		if (chk.dst == -1) {
			feat["last_chunk"] = 1.0;
			feat["no_following_mod"] = 1.0;
		}
		else {
			nlp::chunk chk_mod;
			if (sent.get_chunk_has_mod(chk_mod, chk.id) == true) {
				nlp::token tok = chk_mod.get_token_has_mod();
				if (tok.has_mod && tok.mod.tag["authenticity"] != "") {
					feat["next_authenticity_" + tok.mod.tag["authenticity"]] = 1.0;
				}
			}
			else {
				feat["no_following_mod"] = 1.0;
			}
		}
	}


	void parser::gen_feature_function(nlp::sentence sent, int tok_id, t_feat &feat) {
		std::string func_ex = "";
		std::vector< int > func_ids;

		nlp::chunk chk = sent.get_chunk_by_tokenID(tok_id);
		
		BOOST_FOREACH ( nlp::token tok, chk.tokens ) {
			if (tok_id < tok.id) {
				func_ex += tok.orig;
			}
		}

		feat["func_expression_" + func_ex] = 1.0;
	}


	void parser::gen_feature_basic(nlp::sentence sent, int tok_id, t_feat &feat, int n) {
		BOOST_FOREACH(nlp::chunk chk, sent.chunks) {
			BOOST_FOREACH(nlp::token tok, chk.tokens) {
				if (tok_id <= tok.id + n && tok.id - n <= tok_id && tok_id != tok.id) {
					std::stringstream ss;
					ss << tok.id - tok_id;
					feat["tok_surf_" + ss.str() + "_" + tok.surf] = 1.0;
					feat["tok_orig_" + ss.str() + "_" + tok.orig] = 1.0;
				}
				if (tok.id == tok_id) {
					feat["tok_surf_" + tok.surf] = 1.0;
					feat["tok_orig_" + tok.orig] = 1.0;
				}
			}
		}
	}


	void parser::gen_feature_follow_chunks(nlp::sentence sent, int tok_id, t_feat &feat) {
		nlp::chunk chk_core = sent.get_chunk_by_tokenID(tok_id);
		for (unsigned int i=1 ; i<3 ; ++i) {
			nlp::chunk chk_dep;
			std::stringstream ss;
			ss << i;
			std::string chk_pos = ss.str();
			if (sent.get_dep_chunk(&chk_dep, chk_core)) {
				feat["chk_dep" + chk_pos + "_surf_" + chk_dep.str()] = 1.0;
				feat["chk_dep" + chk_pos + "_orig_" + chk_dep.str_orig()] = 1.0;
				BOOST_FOREACH (nlp::token tok, chk_dep.tokens) {
					feat["chk_dep" + chk_pos + "_tok_surf_" + tok.surf] = 1.0;
					feat["chk_dep" + chk_pos + "_tok_orig_" + tok.orig] = 1.0;
				}
			}
		}
	}


	void parser::gen_feature_ttj(nlp::sentence sent, int tok_id, t_feat &feat) {
		nlp::chunk chk_core = sent.get_chunk_by_tokenID(tok_id);

		int tok_id_start = tok_id;
		std::vector< t_match_func > match_funcs;

		BOOST_FOREACH (nlp::token tok, chk_core.tokens) {
			std::vector< t_match_func > match_funcs_local;
			if (tok.id > tok_id_start) {
				std::string val;
				if (ttjDB.get(tok.surf, &val)) {
					std::vector<std::string> ents;
					boost::algorithm::split(ents, val, boost::algorithm::is_any_of("\t"));
					
					BOOST_FOREACH (std::string ent, ents) {
						std::vector<std::string> seq;
						boost::algorithm::split(seq, ent, boost::algorithm::is_any_of("_"));
						std::string semclass = seq.back();
						seq.pop_back();
						bool match = true;
						t_match_func match_func;
						match_func.tok_ids.push_back(tok.id);

						// functional expression containing only single token
						if (seq.size() == 1 && seq[0] == "") {
							if ( 
									// n(添加) IPA品詞体系では接続詞の一部になるので不可能
									// R(比況) 例文が分からず
									( (semclass == "Q(並立)") && (tok.pos == "助詞" && tok.pos1 == "並立助詞") )
									|| ( (semclass == "O(主体)" || semclass == "N(目的)" || semclass == "b(対象)" || semclass == "d(状況)" || semclass == "e(起点)") && (tok.pos == "助詞" && tok.pos1 == "格助詞") )
									|| ( (semclass == "t(逆接)" || semclass == "r(順接)") && (tok.pos == "助詞" && (tok.pos1 == "接続助詞" || tok.pos1 == "副助詞")) )
									|| ( (semclass == "D(判断)") && (tok.pos == "助動詞" || tok.pos == "名詞") )
									|| ( (semclass == "z(願望)") && (tok.pos == "助動詞" || tok.pos == "動詞") )
									|| ( (semclass == "m(限定)") && (tok.pos == "名詞" || (tok.pos == "助詞" && tok.pos1 == "副助詞")) )
									|| ( (semclass == "f(範囲)") && (tok.pos == "助詞" && tok.pos1 == "副助詞") )
									|| ( (semclass == "v(付帯)") && (tok.pos == "名詞" || (tok.pos == "助詞" && tok.pos1 == "接続助詞")) )
									|| ( (semclass == "I(推量)" || semclass == "A(伝聞)") && (tok.pos == "助動詞" || tok.pos == "動詞") )
									|| ( (semclass == "s(理由)") && (tok.pos == "名詞" || (tok.pos == "助詞" && tok.pos1 == "接続助詞")) )
									|| ( (semclass == "B(過去)" || semclass == "E(可能)") && (tok.pos == "動詞" || tok.pos == "助動詞") )
									|| ( (semclass == "u(対比)" || semclass == "l(強調)") && (tok.pos == "名詞") )
									|| ( (semclass == "J(進行)") && (tok.pos == "動詞" || tok.pos == "名詞" || tok.pos == "接続詞") )
									|| ( (semclass == "P(例示)") && (tok.pos == "名詞" || (tok.pos == "助詞" && (tok.pos1 == "副助詞" || tok.pos1 == "係助詞"))) )
									|| ( (semclass == "o(同時性)") && (tok.pos == "名詞") )
									|| ( (semclass == "G(意志)") && (tok.pos == "名詞" || tok.pos == "助動詞") )
									|| ( semclass == "y(否定)" || semclass == "")
								 ) {
							}
							else {
								match = false;
							}
						}
						// functional expression containing multiple tokens
						else {
							BOOST_FOREACH (std::string s, seq) {
								std::vector<std::string> word;
								boost::algorithm::split(word, s, boost::algorithm::is_any_of(":"));
								int pos = boost::lexical_cast<int>(word[0]);
								std::string surf = word[1];
								
								int around_tid = tok.id + pos;
								if (around_tid < sent.tid_min || sent.tid_max < around_tid) {
									match = false;
									break;
								}

								nlp::token tok = sent.get_token(around_tid);
								if (tok.surf != surf) {
									match = false;
									break;
								}
								match_func.tok_ids.push_back(around_tid);
							}
						}

						if (match) {
							match_func.semrel = semclass;
							sort(match_func.tok_ids.begin(), match_func.tok_ids.end());
							match_funcs_local.push_back(match_func);
						}
					}
				}
			}
			
			unsigned int tok_width = 0;
			BOOST_FOREACH (t_match_func mf, match_funcs_local) {
				if (mf.tok_ids.size() > tok_width) {
					tok_width = mf.tok_ids.size();
				}
			}
			BOOST_FOREACH (t_match_func mf, match_funcs_local) {
				if (mf.tok_ids.size() == tok_width) {
					tok_id_start = mf.tok_ids.back();
					match_funcs.push_back(mf);
				}
			}
		}
		
		std::vector< std::string > sems;
		BOOST_FOREACH (t_match_func mf, match_funcs) {
#ifdef _MODEBUG
			std::cerr << "functional expression\t" << mf.semrel << ":";
			BOOST_FOREACH (int id, mf.tok_ids) {
				std::cerr << " " << sent.get_token(id).surf << "(" << id << ")";
			}
			std::cerr << std::endl;
#endif
			sems.push_back(mf.semrel);
		}
		sort(sems.begin(), sems.end());
		sems.erase(unique(sems.begin(), sems.end()), sems.end());
		BOOST_FOREACH(std::string sem, sems) {
			feat[sem] = 1.0;
		}
	}


	void parser::gen_feature_ttj_orig(nlp::sentence sent, int tok_id, t_feat &feat) {
		nlp::chunk chk_core = sent.get_chunk_by_tokenID(tok_id);

		std::vector< std::string > sems;
		std::vector<nlp::token>::reverse_iterator rit_tok;
		for (rit_tok=(chk_core.tokens).rbegin() ; rit_tok!=(chk_core.tokens).rend() ; ++rit_tok) {
			if (rit_tok->id == tok_id) {
				break;
			}

			std::string val;
			if (ttjDB.get(rit_tok->orig, &val)) {
				std::vector< std::string > ents;
				boost::algorithm::split(ents, val, boost::algorithm::is_any_of("\t"));
//				std::cout << rit_tok->orig << "->" << val << std::endl;

				BOOST_FOREACH(std::string ent, ents) {
//					std::cout << rit_tok->orig << "->" << ent << std::endl;
					bool match = true;
					std::vector< std::string > seq;
					boost::algorithm::split(seq, ent, boost::algorithm::is_any_of("_"));
					std::string semclass = seq.back();
					seq.pop_back();
					if (seq[0] == "") {
//						std::cout << rit_tok->orig << "->" << ent << std::endl;
					}
					else {
						BOOST_FOREACH (std::string s, seq) {
							std::vector< std::string > word;
							boost::algorithm::split(word, s, boost::algorithm::is_any_of(":"));
							int pos = boost::lexical_cast<int>(word[0]);
							std::string surf = word[1];

							int around_tid = rit_tok->id + pos;
							if (around_tid < sent.tid_min || sent.tid_max < around_tid) {
								match = false;
								break;
							}
							nlp::token tok = sent.get_token(around_tid);
							if (tok.surf != surf) {
								match = false;
								break;
							}
						}
					}

					if (match) {
						sems.push_back(semclass);
//						std::cout << rit_tok->orig << "->" << ent << std::endl;
					}
				}
			}
		}
		
		sort(sems.begin(), sems.end());
		sems.erase(unique(sems.begin(), sems.end()), sems.end());
		BOOST_FOREACH(std::string sem, sems) {
			feat[sem] = 1.0;
		}
//		std::cout << chk_core.str() << "->" << join(sems, ",") << std::endl;
	}

	

	void parser::gen_feature_fadic(nlp::sentence sent, int tok_id, t_feat &feat) {
		nlp::token tok_core = sent.get_token(tok_id);
		nlp::chunk chk_core = sent.get_chunk_by_tokenID(tok_id);
		
		std::string tense, auth;
		
		if (tok_core.has_mod) {
			if (tok_core.mod.tag["tense"] == "未来") {
				tense = "future";
			}
			else if (tok_core.mod.tag["tense"] == "非未来") {
				tense = "present";
			}
			else {
				tense = "";
			}
		}
#ifdef _MODEBUG
		std::cerr << " lookup fadic: " << tok_core.orig << " tense -> " << tense << std::endl;
#endif

		nlp::token tok_dst;
		if (chk_core.dst == -1) {
		}
		else {
			nlp::chunk chk_dst = sent.get_chunk(chk_core.dst);
			tok_dst = chk_dst.get_token_has_mod();
			if (tok_dst.orig != "*") {
				if (tok_dst.mod.tag["authenticity"] == "成立" || tok_dst.mod.tag["authenticity"] == "高確率" || tok_dst.mod.tag["authenticity"] == "不成立から成立" || tok_dst.mod.tag["authenticity"] == "低確率から高確率") {
					auth = "pos";
				}
				else if (tok_dst.mod.tag["authenticity"] == "0") {
					auth = "";
				}
				else {
					auth = "neg";
				}
#ifdef _MODEBUG
				std::cerr << " lookup fadic: " << tok_core.orig << " auth " << "(" << tok_dst.orig << ")" << " -> " << auth << std::endl;
#endif
			}
		}
		
		if (auth != "" && tense != "") {
			std::string key = tok_dst.orig + ":" + auth + "_" + tense + "_actuality";
#ifdef _MODEBUG
			std::cerr << " lookup fadic: " << key;
#endif
			std::string val;
			if (fadicDB.get(key, &val)) {
#ifdef _MODEBUG
				std::cerr << " -> " << val << std::endl;
#endif
				feat["fadic_authenticity_" + val] = 1.0;
			}
			else {
#ifdef _MODEBUG
				std::cerr << " -> none" << std::endl;
#endif
				feat["fadic_noent"] = 1.0;
			}
		}
	}

};

