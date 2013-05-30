#include <stdlib.h>
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

	void parser::gen_feature(nlp::sentence sent, int tok_id, t_feat &feat) {
		gen_feature_basic(sent, tok_id, feat, 3);
		gen_feature_function(sent, tok_id, feat);
		gen_feature_follow_mod(sent, tok_id, feat);
		gen_feature_ttj(sent, tok_id, feat);
		gen_feature_fadic(sent, tok_id, feat);
	}


	void parser::gen_feature_follow_mod(nlp::sentence sent, int tok_id, t_feat &feat) {
		nlp::chunk chk;
		chk = sent.get_chunk_by_tokenID(tok_id);
		
		if (chk.dst == -1) {
			feat["last_chunk"] = 1.0;
		}
		else {
			nlp::chunk chk_mod;
			if (sent.get_chunk_has_mod(chk_mod, chk.id) == true) {
				nlp::token tok = chk_mod.get_token_has_mod();
				if (tok.mod.tag.find("actuality") != tok.mod.tag.end()) {
					feat["next_actuality_" + tok.mod.tag["actuality"]] = 1.0;
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


	void parser::gen_feature_ttj(nlp::sentence sent, int tok_id, t_feat &feat) {
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
				boost::algorithm::split(ents, val, boost::algorithm::is_space());
//				std::cout << rit_tok->orig << "->" << val << std::endl;

				BOOST_FOREACH(std::string ent, ents) {
//					std::cout << rit_tok->orig << "->" << ent << std::endl;
					bool match = true;
					std::vector< std::string > seq;
					boost::algorithm::split(seq, ent, boost::algorithm::is_any_of("_"));
					std::string semclass = seq[0];
					if (seq[1] == "") {
//						std::cout << rit_tok->orig << "->" << ent << std::endl;
					}
					else {
						for (unsigned int i=1 ; i<seq.size() ; ++i) {
							std::vector< std::string > word;
							boost::algorithm::split(word, seq[i], boost::algorithm::is_any_of(":"));
							int pos = boost::lexical_cast<int>(word[0]);
							std::string surf = word[1];

							int around_tid = tok_id + pos;
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
		nlp::chunk chk_core = sent.get_chunk_by_tokenID(tok_id);
		nlp::chunk chk_mod;
		if (sent.get_chunk_has_mod(chk_mod, chk_core.id) == true) {
			nlp::token tok = chk_mod.get_token_has_mod();
			std::string key = tok.orig + ":";
			if (tok.mod.tag.find("actuality") != tok.mod.tag.end()) {
				key += "pos_present_actuality";
			}
			else {
				key += "neg_present_actuality";
			}
			
			std::string val;
			if (fadicDB.get(key, &val)) {
				feat["fadic_actuality_" + val] = 1.0;
			}
			else {
				feat["fadic_noent"] = 1.0;
			}
		}
	}

};

