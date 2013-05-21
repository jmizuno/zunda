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

	bool parser::gen_feature(nlp::sentence sent, int tok_id, t_feat &feat) {
		gen_feature_basic(sent, tok_id, feat, 3);
		gen_feature_function(sent, tok_id, feat);
		gen_feature_follow_mod(sent, tok_id, feat);

		return true;
	}


	bool parser::gen_feature_follow_mod(nlp::sentence sent, int tok_id, t_feat &feat) {
		nlp::chunk chk;
		chk = sent.get_chunk_by_tokenID(tok_id);
		
		if (chk.dst == -1) {
			feat["last_chunk"] = 1.0;
			return true;
		}

		chk = sent.get_chunk(chk.dst);
		while (chk.has_mod == false) {
			if (chk.dst == -1) {
				feat["no_following_mod"] = 1.0;
				return true;
			}
			chk = sent.get_chunk(chk.dst);
		}
		
		nlp::token tok = chk.get_token_has_mod();
		if (tok.mod.tag.find("actuality") != tok.mod.tag.end()) {
			feat["next_actuality_" + tok.mod.tag["actuality"]] = 1.0;
		}

		return true;
	}


	bool parser::gen_feature_function(nlp::sentence sent, int tok_id, t_feat &feat) {
		std::string func_ex = "";
		std::vector< int > func_ids;

		nlp::chunk chk = sent.get_chunk_by_tokenID(tok_id);
		
		BOOST_FOREACH ( nlp::token tok, chk.tokens ) {
			if (tok_id < tok.id) {
				func_ex += tok.orig;
			}
		}

		feat["func_expression_" + func_ex] = 1.0;
		
		return true;
	}


	bool parser::gen_feature_basic(nlp::sentence sent, int tok_id, t_feat &feat, int n) {
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
		return true;
	}

};

