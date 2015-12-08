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

#include "sentence.hpp"
#include "modality.hpp"
#include "util.hpp"


namespace modality {
	void feature_generator2::gen_feature_mod(const std::string &tag) {
		if (tok_core->has_mod) {
			feat_cat["mod_" + tag][tok_core->mod.tag[tag]] = 1.0;
		}
	}


	bool feature_generator2::compile_feat_str(const std::vector<std::string> &use_feats, std::string &str_res) {
		std::stringstream feat_str;
		t_feat compiled_feat;
		compile_feat(use_feats, compiled_feat);
		for (t_feat::iterator it=compiled_feat.begin() ; it!=compiled_feat.end() ; ++it) {
			feat_str << it->first << ":" << it->second << " ";
		}
		str_res = feat_str.str();
		return true;
	}


	bool feature_generator2::compile_feat(const std::vector<std::string> &use_feats, t_feat &compiled_feat) {
		compiled_feat.clear();
		BOOST_FOREACH (std::string cat, use_feats) {
			t_feat::iterator it_feat;
			if (feat_cat.find(cat) != feat_cat.end()) {
				for (it_feat=feat_cat[cat].begin() ; it_feat!=feat_cat[cat].end() ; ++it_feat) {
					compiled_feat[cat + "_" + it_feat->first] = it_feat->second;
				}
			}
			else {
#ifdef _MODEBUG
//				std::cerr << "WARN: no such feature \"" << cat << "\"" << std::endl;
#endif
			}
		}
		return true;
	}


	void feature_generator2::gen_feature_neg() {
		BOOST_FOREACH (nlp::token t, chk_core->tokens) {
			if (t.id > tok_core->id) {
				if (std::binary_search(keyterms["Negations"].begin(), keyterms["Negations"].end(), t.orig) || std::binary_search(keyterms["Correctings"].begin(), keyterms["Correctings"].end(), t.orig) ) {
					feat_cat["neg"]["exist_in_core_chunk"] = 1.0;
					feat_cat["neg"]["term_in_core_chunk_" + t.orig] = 1.0;
				}
			}
		}

		unsigned int tid_a_core = chk_core->tokens[chk_core->tokens.size()-1].id+1;
		for (unsigned int tid=tid_a_core ; tid<tid_a_core+8 ; ++tid) {
			nlp::token *t = sent->get_token(tid);
			if (t != NULL) {
				if (std::binary_search(keyterms["Negations"].begin(), keyterms["Negations"].end(), t->orig) || std::binary_search(keyterms["Correctings"].begin(), keyterms["Correctings"].end(), t->orig) ) {
					feat_cat["neg"]["exist_after_core"] = 1.0;
					feat_cat["neg"]["term_after_core_" + t->orig] = 1.0;
				}
			}
		}
	}


	void feature_generator2::gen_feature_function() {
		std::vector< std::vector< nlp::token * > > ng_tok;
		std::vector< nlp::token * > toks;
		for (unsigned int tid=tok_core->id+1 ; tid<tok_core->id+8 ; ++tid) {
			nlp::token *t = sent->get_token(tid);
			if (t!=NULL)
				toks.push_back(t);
		}
		if (toks.size() != 0) {
			ng_tok.clear();
			get_subvec(&ng_tok, toks, 1, toks.size());
			BOOST_FOREACH (std::vector<nlp::token *> ngt, ng_tok) {
				std::string f_surf_str, f_orig_str;
				std::vector<std::string> f_surf_v, f_orig_v;
				BOOST_FOREACH (nlp::token *t, ngt) {
					f_surf_v.push_back(t->surf);
					f_orig_v.push_back(t->orig);
				}
				join(f_surf_str, f_surf_v, ".");
				join(f_orig_str, f_orig_v, ".");
				feat_cat["func"]["surf_" + f_surf_str] = 1.0;
				feat_cat["func"]["orig_" + f_orig_str] = 1.0;
			}
		}
	}


	void feature_generator2::gen_feature_basic(const int n) {
		BOOST_FOREACH(nlp::chunk chk, sent->chunks) {
			BOOST_FOREACH(nlp::token tok, chk.tokens) {
				if (tok_core->id <= tok.id + n && tok.id - n <= tok_core->id && tok_core->id != tok.id) {
					std::stringstream ss;
					ss << tok.id - tok_core->id;
					feat_cat["tok"]["surf_" + ss.str() + "_" + tok.surf] = 1.0;
					feat_cat["tok"]["orig_" + ss.str() + "_" + tok.orig] = 1.0;
				}
				if (tok.id == tok_core->id) {
					feat_cat["tok"]["surf_" + tok.surf] = 1.0;
					feat_cat["tok"]["orig_" + tok.orig] = 1.0;
				}
			}
		}
	}


	void feature_generator2::gen_feature_dst_chunks() {
		nlp::chunk *chk_dst;
		chk_dst = sent->get_dst_chunk(*chk_core);
		for (unsigned int i=1 ; i<3 ; ++i) {
			if (chk_core->dst != -1) {
				std::string chk_str;
				chk_dst->str(chk_str);
				feat_cat["chunk"]["dst_surf_" + chk_str] = 1.0;
				chk_dst->str_orig(chk_str);
				feat_cat["chunk"]["dst_orig_" + chk_str] = 1.0;
				int tid = 0;
				BOOST_FOREACH (nlp::token tok, chk_dst->tokens) {
					std::stringstream ss;
					ss << tid;
					feat_cat["chunk"]["dst_tok_surf_" + ss.str() + "_" + tok.surf] = 1.0;
					feat_cat["chunk"]["dst_tok_orig_" + ss.str() + "_" + tok.orig] = 1.0;
					tid++;
				}
			}
		}
	}


	void feature_generator2::gen_feature_fsem() {
		std::vector<std::string> fsems;
		BOOST_FOREACH (nlp::token tok, chk_core->tokens) {
			if (tok.fsem.compare(0, 2, "B:") == 0) {
				std::string fsem = tok.fsem;
				fsem.erase(0, 2);
				fsems.push_back(fsem);
			}
		}
		std::vector< std::vector<std::string> > ng_fsem;
		get_subvec(&ng_fsem, fsems, 1, fsems.size());
		BOOST_FOREACH (std::vector<std::string> fsems, ng_fsem) {
			std::string _fsem_str;
			join(_fsem_str, fsems, ".");
			feat_cat["func_sem"][_fsem_str] = 1.0;
		}
	}


	void feature_generator2::gen_feature_ttj(cdbpp::cdbpp *dbr_ttj) {
		int tok_id_start = tok_core->id;
		std::vector< t_match_func > match_funcs;

		BOOST_FOREACH (nlp::token tok, chk_core->tokens) {
			std::vector< t_match_func > match_funcs_local;
			if (tok.id > tok_id_start) {
				size_t vsize;
				const char *value = (const char *)dbr_ttj->get(tok.surf.c_str(), tok.surf.length(), &vsize);
				if (value != NULL) {
					std::string val = std::string(value, vsize);
					std::vector<std::string> ents;
					boost::algorithm::split(ents, val, boost::algorithm::is_any_of("\t"));

					BOOST_FOREACH (std::string ent, ents) {
						std::vector<std::string> seq;
						boost::algorithm::split(seq, ent, boost::algorithm::is_any_of("_"));
						std::string semclass = seq.back();
						if (semclass == "") {
							continue;
						}
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
								if (around_tid < sent->tid_min || sent->tid_max < around_tid) {
									match = false;
									break;
								}

								nlp::token *tok = sent->get_token(around_tid);
								if (tok->surf != surf) {
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
			std::cerr << " lookup ttj: " << mf.semrel << ":";
			BOOST_FOREACH (int id, mf.tok_ids) {
				std::cerr << " " << sent->get_token(id)->surf << "(" << id << ")";
			}
			std::cerr << std::endl;
#endif
			sems.push_back(mf.semrel);
		}
		sort(sems.begin(), sems.end());
		sems.erase(unique(sems.begin(), sems.end()), sems.end());
		BOOST_FOREACH(std::string sem, sems) {
			feat_cat["func_sem"][sem] = 1.0;
		}
	}



	void feature_generator2::gen_feature_fadic(cdbpp::cdbpp *dbr_fadic) {
		std::string tense, auth;

		if (tok_core->has_mod) {
			if (tok_core->mod.tag["tense"] == "未来") {
				tense = "future";
			}
			else if (tok_core->mod.tag["tense"] == "非未来") {
				tense = "present";
			}
			else {
				tense = "";
			}
		}
#ifdef _MODEBUG
		std::cerr << " lookup fadic: " << tok_core->orig << " tense -> " << tense << std::endl;
#endif

		nlp::token *tok_dst;
		if (chk_core->dst != -1) {
			nlp::chunk *chk_dst = sent->get_chunk(chk_core->dst);
			tok_dst = chk_dst->get_token_has_mod();
			if (tok_dst != NULL) {
				if (tok_dst->mod.tag["authenticity"] == "成立" || tok_dst->mod.tag["authenticity"] == "高確率" || tok_dst->mod.tag["authenticity"] == "不成立から成立" || tok_dst->mod.tag["authenticity"] == "低確率から高確率") {
					auth = "pos";
				}
				else if (tok_dst->mod.tag["authenticity"] == "0") {
					auth = "";
				}
				else {
					auth = "neg";
				}
#ifdef _MODEBUG
				std::cerr << " lookup fadic: " << tok_core->orig << " auth " << "(" << tok_dst->orig << ")" << " -> " << auth << std::endl;
#endif
			}
		}

		if (auth != "" && tense != "") {
			boost::unordered_map<std::string, std::string> keys;
			keys["authenticity"] = tok_dst->orig + ":" + auth + "_" + tense + "_actuality";
			keys["sentiment"] = tok_dst->orig + ":" + auth + "_" + tense + "_sentiment";
			keys["worth"] = tok_dst->orig + ":" + auth + "_" + tense + "_worth";

			for (boost::unordered_map<std::string, std::string>::iterator it=keys.begin() ; it!=keys.end() ; ++it) {
#ifdef _MODEBUG
				std::cerr << " lookup fadic: " << it->first;
#endif
				size_t vsize;
				const char *value = (const char *)dbr_fadic->get(it->second.c_str(), it->second.length(), &vsize);
				if (value != NULL) {
					std::string val = std::string(value, vsize);
#ifdef _MODEBUG
					std::cerr << " -> " << val << std::endl;
#endif
					feat_cat["fadic_" + it->first][val] = 1.0;
				}
				else {
#ifdef _MODEBUG
					std::cerr << " -> none" << std::endl;
#endif
//					feat_cat["fadic_" + it->first]["noent"] = 1.0;
				}
			}
		}
	}
};


