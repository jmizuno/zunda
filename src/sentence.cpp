#include <cstdlib>
#include <iostream>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <iomanip>

#include "sentence.hpp"
#include "util.hpp"


void rm_quote(std::string &str) {
	str.replace(0, 1, "");
	str.replace(str.size()-1, 1, "");
}

namespace nlp {
	void modality::parse(const std::string &mod_line) {
		std::vector< std::string > l;
		boost::algorithm::split(l, mod_line, boost::algorithm::is_any_of("\t"));

		if (l.size() != 9) {
			std::cerr << "error: invalid modality tag format: " << mod_line << std::endl;
		}
		else {
			std::vector< std::string > tid_strs;
			boost::algorithm::split(tid_strs, l[1], boost::algorithm::is_any_of(","));
			BOOST_FOREACH(std::string tid_str, tid_strs) {
				tids.push_back(boost::lexical_cast<int>(tid_str));
			}

			tag["source"] = l[2];
			tag["tense"] = l[3];
			tag["assumptional"] = l[4];
			tag["type"] = l[5];
			tag["authenticity"] = l[6];
			tag["sentiment"] = l[7];
			tag["focus"] = l[8];
		}
	}

	void modality::str(std::string &str) {
		std::string tid_str;
		join(tid_str, tids, ",");

		str = tid_str + "\t" + tag["source"] + "\t" + tag["tense"] + "\t" + tag["assumptional"] + "\t" + tag["type"] + "\t" + tag["authenticity"] + "\t" + tag["sentiment"] + "\t" + tag["focus"];
	}
};


namespace nlp {
	bool pas::is_pred() {
		if (pred_type == "null") {
			return false;
		}
		else {
			return true;
		}
	}

	bool pas::parse(const std::string &pas_line, pas *pas) {
		std::vector<std::string> pas_sets;
		boost::algorithm::split(pas_sets, pas_line, boost::algorithm::is_any_of("|") );

		BOOST_FOREACH (std::string pas_set, pas_sets) {
			boost::algorithm::trim(pas_set);
			std::vector<std::string> pas_infos;
			boost::algorithm::split(pas_infos, pas_set, boost::algorithm::is_any_of(" ") );

			BOOST_FOREACH(std::string pas_info, pas_infos) {
				std::vector<std::string> v;
				boost::algorithm::split(v, pas_info, boost::algorithm::is_any_of("=") );

				if (v.size() == 2) {
					rm_quote(v[1]);
					if (v[0] == "type") {
						pas->pred_type = v[1];
					}
					else if (v[0] == "ID") {
						if (pas->arg_id == -1)
							pas->arg_id = boost::lexical_cast<int>(v[1]);
					}
					else {
						if (pas->arg_type.find(v[0]) == pas->arg_type.end())
							pas->arg_type[v[0]] = boost::lexical_cast<int>(v[1]);
					}
				}
			}
		}

		return true;
	}
};

namespace nlp {
	bool token::parse_mecab(const std::string &line, const int tok_id) {
		std::vector<std::string> tok_infos;
		boost::algorithm::split(tok_infos, line, boost::algorithm::is_any_of("\t") );

		std::vector<std::string> v;
		boost::algorithm::split(v, tok_infos[1], boost::is_any_of(","));

		id = tok_id;
		surf = tok_infos[0];
		pos = v[0];
		pos1 = v[1];
		pos2 = v[2];
		pos3 = v[3];
		type = v[4];
		form = v[5];
		orig = v[6];
		if (v.size() > 7) {
			read = v[7];
			pron = v[8];
		}
		else {
			read = tok_infos[0];
		}

		if (tok_infos.size() > 2)
			ne = tok_infos[2];

		if (tok_infos.size() > 3)
			pas.parse(tok_infos[3], &pas);

		return true;
	}


	bool token::parse_mecab_juman(const std::string &line, const int tok_id) {
		std::vector<std::string> tok_infos;
		boost::algorithm::split(tok_infos, line, boost::algorithm::is_any_of("\t") );

		std::vector<std::string> v;
		boost::algorithm::split(v, tok_infos[1], boost::algorithm::is_any_of(","));

		id = tok_id;
		surf = tok_infos[0];

		pos = v[0];
		orig = v[4];
		read = v[5];
		if (pos.compare(pos.size()-judge_pos_juman.size(), pos.size(), judge_pos_juman) == 0) {
			pos1 = v[1];
			pos2 = v[2];
			pos3 = v[3];
		}
		else {
			form = v[1];
			type = v[2];
			form2 = v[3];
		}

		if (tok_infos.size() > 2)
			ne = tok_infos[2];

		if (tok_infos.size() > 3)
			pas.parse(tok_infos[3], &pas);

		return true;
	}


	bool token::parse_mecab_unidic(const std::string &line, const int tok_id) {
		std::vector<std::string> tok_infos;
		boost::algorithm::split(tok_infos, line, boost::algorithm::is_any_of("\t"));

		std::vector<std::string> v;
		boost::algorithm::split(v, tok_infos[1], boost::algorithm::is_any_of(","));

		id = tok_id;
		surf = tok_infos[0];
		pos1 = v[0];
		pos = pos1;
		pos2 = v[1];
		pos3 = v[2];
		pos4 = v[3];
		cType = v[4];
		cForm = v[5];
		lForm = v[6];
		lemma = v[7];
		orig = lemma;
		orth = v[8];
		pron = v[9];
		orthBase = v[10];
		pronBase = v[11];
		goshu = v[12];
		iType = v[13];
		iForm = v[14];
		fType = v[15];
		fForm = v[16];

		if (tok_infos.size() > 2)
			ne = tok_infos[2];

		if (tok_infos.size() > 3)
			pas.parse(tok_infos[3], &pas);

		return true;
	}


	bool token::parse_juman(const std::string &line, const int tok_id) {
		std::vector<std::string> l;
		boost::algorithm::split(l, line, boost::is_any_of(" "));

		id = tok_id;
		surf = l[0];
		read = l[1];
		orig = l[2];

		pos = l[3];
		pos_id = boost::lexical_cast<int>(l[4]);
		/* when pos token */
		if (pos.compare(pos.size()-judge_pos_juman.size(), pos.size(), judge_pos_juman) == 0) {
			pos1 = l[5];
			pos1_id = boost::lexical_cast<int>(l[6]);
			pos2 = l[7];
			pos2_id = boost::lexical_cast<int>(l[8]);
			pos3 = l[9];
			pos3_id = boost::lexical_cast<int>(l[10]);
		}
		else {
			form = l[5];
			form_id = boost::lexical_cast<int>(l[6]);
			type = l[7];
			type_id = boost::lexical_cast<int>(l[8]);
			form2 = l[9];
			form2_id = boost::lexical_cast<int>(l[10]);
		}

		/* Daihyo Hyoki */
		if (l.size() > 11) {
			if (l[11] == "NIL") {
				sem_info = l[11];
			}
			else {
				std::vector<std::string> sems;
				for (unsigned int i=11 ; i<l.size() ; ++i) {
					sems.push_back(l[i]);
				}
				join(sem_info, sems, " ");
			}
		}

		return true;
	}
};


namespace nlp {
	void chunk::str(std::string &chk_str) {
		chk_str.clear();
		BOOST_FOREACH(token tok, tokens) {
			chk_str += tok.surf;
		}
	}


	void chunk::str_orig(std::string &chk_str) {
		chk_str.clear();
		BOOST_FOREACH(token tok, tokens) {
			chk_str += tok.orig;
		}
	}


	token* chunk::get_token_has_mod() {
		std::vector<token>::reverse_iterator rit_tok;
		for (rit_tok=tokens.rbegin() ; rit_tok!=tokens.rend() ; ++rit_tok) {
			if (rit_tok->has_mod) {
				return &(*rit_tok);
			}
		}
		return NULL;
	}
};


namespace nlp {
	void sentence::str(std::string &str_res, const std::string &delimiter) {
		std::string chk_str;
		std::stringstream ss;
		bool first_flag = true;
		BOOST_FOREACH (chunk chk, chunks) {
			if (first_flag) {
				first_flag = false;
			}
			else {
				ss << delimiter;
			}
			chk.str(chk_str);
			ss << chk_str;
		}
		str_res = ss.str();
	}


	void sentence::str(std::string &str_res) {
		str(str_res, "");
	}


	bool sentence::parse(const std::string &str) {
		std::vector< std::string > buf;
		boost::algorithm::split(buf, str, boost::algorithm::is_any_of("\n") );
		return parse(buf);
	}


	bool sentence::parse(const std::vector< std::string > &lines) {
		std::vector<std::string> input_orig_lines;
		BOOST_FOREACH (std::string line, lines) {
			if (line.compare(0, 6, "#EVENT") != 0 && line.compare(0, 8, "#FUNCEXP") != 0) {
				input_orig_lines.push_back(line);
			}
		}
		input_orig.clear();
		join(input_orig, input_orig_lines, "\n");  // stored original parsed sentence

		std::vector< modality > mods;
		std::vector< std::string > funcexps;

		BOOST_FOREACH (std::string l, lines) {
			if (l.compare(0, 6, "# S-ID") == 0) {
				std::vector<std::string> buf;
				boost::algorithm::split(buf, l, boost::algorithm::is_any_of(" "));
				if (buf[2].substr(buf[2].size()-1, buf[2].size()) == ";") {
					sent_id = buf[2].substr(0, buf[2].size()-1);
				}
				else {
					sent_id = buf[2];
				}
			}
			else if (l.compare(0, 6, "#EVENT") == 0) {
				modality mod;
				mod.parse(l);
				mods.push_back(mod);
			}
			else if (l.compare(0, 8, "#FUNCEXP") == 0) {
				std::string sub = l.substr(9);
				boost::algorithm::split(funcexps, sub, boost::algorithm::is_any_of(","));
			}
			else if (l.compare(0, 1, "#") == 0) {
			}
			else {
				break;
			}
		}

		switch (da_tool) {
			case CaboCha:
			case JDepP:
				if (!parse_cabocha(lines)) {
					return false;
				}
				break;
			case KNP:
				if (!parse_knp(lines)) {
					return false;
				}
				break;
			default:
				std::cerr << "ERROR: invalid dependency parsing tool" << std::endl;
				return false;
		}

		if (funcexps.size() == (unsigned int)tid_max + 1) {
			has_fsem = true;
		}

		std::vector<chunk>::iterator it_chk;
		std::vector<token>::iterator it_tok;
		for (it_chk = chunks.begin() ; it_chk != chunks.end() ; ++it_chk) {
			for (it_tok = it_chk->tokens.begin() ; it_tok != it_chk->tokens.end() ; ++it_tok) {
				if (has_fsem) {
					it_tok->has_fsem = true;
					it_tok->fsem = funcexps[it_tok->id];
				}

				BOOST_FOREACH (modality mod, mods) {
					if ( find(mod.tids.begin(), mod.tids.end(), it_tok->id) != mod.tids.end() ) {
						it_tok->mod = mod;
						it_tok->has_mod = true;
					}
				}
			}
		}
		return true;
	}


	bool sentence::parse_knp(const std::vector< std::string > &lines) {
		int tok_cnt = 0;
		int chk_cnt = 0;
		int tok_cnt_local = 0;
		bool comment_flag = true;
		std::string line;

		BOOST_FOREACH( line, lines) {
			if (comment_flag && line.compare(0, 1, "#") == 0) {
			}
			else if (line.compare(0, 2, "* ") == 0) {
				comment_flag = false;
				chunk chk;
				char type;
				sscanf(line.c_str(), "* %d%c", &chk.dst, &type);
				chk.id = chk_cnt;
				chk.type = type;
				chk_cnt++;
				tok_cnt_local = 0;

				chunks.push_back(chk);
			}
			else if (line.compare(0, 2, "+ ") == 0 ) {
			}
			else if (line.compare(0, 3, "EOS") == 0) {
				break;
			}
			else {
				token tok;
				tok.parse_juman(line, tok_cnt);
				t2c[tok.id] = chunks[chunks.size()-1].id;
				chunks[chunks.size()-1].tok_g2l[tok_cnt] = tok_cnt_local;
				chunks[chunks.size()-1].tokens.push_back(tok);

				tok_cnt++;
				tok_cnt_local++;
			}
		}

		tid_min = 0;
		tid_max = tok_cnt-1;
		cid_min = 0;
		cid_max = chunks.size()-1;

		return true;
	}


	bool sentence::parse_cabocha(const std::vector< std::string > &lines) {
		int tok_cnt = 0;
		int tok_cnt_local = 0;
		bool comment_flag = true;
		std::string line;

		BOOST_FOREACH( line, lines) {
			if (comment_flag && line.compare(0, 1, "#") == 0) {
			}
			else if (line.compare(0, 2, "* ") == 0) {
				comment_flag = false;
				chunk chk;
				char type;
				sscanf(line.c_str(), "* %d %d%c", &chk.id, &chk.dst, &type);
				chk.type = type;
				tok_cnt_local = 0;

				if (chk.id != (int)chunks.size()) {
					std::cerr << "error: chunk id is not in order" << std::endl;
					return false;
				}

				chunks.push_back(chk);
			}
			else if (line.compare(0, 3, "EOS") == 0) {
				break;
			}
			else {
				token tok;
				switch (ma_dic) {
					case IPADic:
						tok.parse_mecab(line, tok_cnt);
						break;
					case JumanDic:
						tok.parse_mecab_juman(line, tok_cnt);
						break;
					case UniDic:
						tok.parse_mecab_unidic(line, tok_cnt);
						break;
					default:
						std::cerr << "Error: unknown Morphological Analysis tool" << std::endl;
						return false;
				}

				t2c[tok.id] = chunks[chunks.size()-1].id;
				chunks[chunks.size()-1].tok_g2l[tok_cnt] = tok_cnt_local;
				chunks[chunks.size()-1].tokens.push_back(tok);

				tok_cnt++;
				tok_cnt_local++;
			}
		}

#if _MODEBUG
		for (int ti=0 ; ti<tok_cnt-1 ; ++ti) {
			std::cout << ti << "->" << t2c[ti] << std::endl;
		}
#endif

		tid_min = 0;
		tid_max = tok_cnt-1;
		cid_min = 0;
		cid_max = chunks.size()-1;

		std::vector<chunk>::iterator it_chk;
		std::vector<token>::iterator it_tok;
		for (it_chk = chunks.begin() ; it_chk != chunks.end() ; ++it_chk) {
			for (it_tok = it_chk->tokens.begin() ; it_tok != it_chk->tokens.end() ; ++it_tok) {
				boost::unordered_map<std::string, int>::iterator it;
				for (it = it_tok->pas.arg_type.begin() ; it != it_tok->pas.arg_type.end() ; ++it) {
					BOOST_FOREACH(chunk chk, chunks) {
						BOOST_FOREACH(token tok, chk.tokens) {
							if (it->second == tok.pas.arg_id) {
								it_tok->pas.args[it->first] = tok.id;
							}
						}
					}
				}
			}
		}

		return true;
	}


	chunk* sentence::get_chunk(const int cid) {
		if (cid_min <= cid && cid <= cid_max) {
			return &chunks[cid];
		}
		else {
			return NULL;
		}
	}


	chunk* sentence::get_chunk_by_tokenID(const int tid) {
		if (t2c.find(tid) != t2c.end() ) {
			return get_chunk(t2c[tid]);
		}
		else {
			return NULL;
		}
	}


	token* sentence::get_token(const int tid) {
		if (tid_min <= tid && tid <= tid_max) {
			nlp::chunk *chk = get_chunk(t2c[tid]);
			return &(chk->tokens[chk->tok_g2l[tid]]);
		}
		return NULL;
	}


	chunk* sentence::get_dst_chunk(const chunk &chk_core) {
		if (chk_core.dst != -1 && cid_min <= chk_core.dst && chk_core.dst <= cid_max) {
			return get_chunk(chk_core.dst);
		}
		return NULL;
	}


/*	chunk* sentence::get_dst_chunk(const chunk &chk_core, const unsigned int depth) {
		int cid_dst = chk_core.dst;
		if (cid_dst == -1) {
			return NULL;
		}
		chunk *chk;
		for (unsigned int i=0 ; i<depth ; ++i) {
			chk = get_chunk(cid_dst);
			if (chk != NULL) {
				cid_dst = chk->dst;
			}
			else {
				return chk;
			}
		}
		return NULL;
	}
	*/


	void sentence::cabocha(std::string &str_res) {
		std::stringstream cabocha_ss;
		int eve_id = 0;
		BOOST_FOREACH ( chunk chk, chunks ) {
			BOOST_FOREACH ( token tok, chk.tokens) {
				if (tok.has_mod) {
					std::string mod_str;
					tok.mod.str(mod_str);
					cabocha_ss << "#EVENT" << eve_id << "\t" << mod_str << "\n";
					eve_id++;
				}
			}
		}

		if (has_fsem) {
			cabocha_ss << "#FUNCEXP\t";
			BOOST_FOREACH (chunk chk, chunks) {
				BOOST_FOREACH (token tok, chk.tokens) {
					cabocha_ss << tok.fsem;
					if (tok.id != tid_max)
						cabocha_ss << ",";
				}
			}
			cabocha_ss << "\n";
		}

		BOOST_FOREACH( chunk chk, chunks ) {
			cabocha_ss << "* " << chk.id << " " << chk.dst << chk.type << " " << chk.subj << "/" << chk.func << " " << std::showpoint << std::setprecision(7) << chk.score << "\n";
			BOOST_FOREACH ( token tok, chk.tokens ) {
				cabocha_ss << tok.surf << "\t" << tok.pos << "," << tok.pos1 << "," << tok.pos2 << "," << tok.pos3 << "," << tok.type << "," << tok.form << "," << tok.orig << "," << tok.read << "," << tok.pron << "\t" << tok.ne;

				if (tok.pas.is_pred() || tok.pas.arg_id != -1) {
					std::vector< std::string > pas_info;
					if (tok.pas.is_pred()) {
						pas_info.push_back("type=\"" + tok.pas.pred_type + "\"");
					}
					if (tok.pas.arg_id != -1) {
						std::stringstream ss;
						ss << "ID=\"" << tok.pas.arg_id << "\"";
						pas_info.push_back(ss.str());
					}
					boost::unordered_map< std::string, int >::iterator it_arg_type;
					for (it_arg_type=tok.pas.arg_type.begin() ; it_arg_type!=tok.pas.arg_type.end() ; ++it_arg_type) {
						std::stringstream ss;
						ss << it_arg_type->first << "=\"" << it_arg_type->second << "\"";
						pas_info.push_back(ss.str());
					}

					std::string pas_info_str;
					join(pas_info_str, pas_info, " ");
					cabocha_ss << "\t" << pas_info_str;
				}
				cabocha_ss << "\n";
			}
		}
		cabocha_ss << "EOS";

		str_res = cabocha_ss.str();
	}


	bool sentence::pp() {
		std::cout << sent_id << std::endl;
		BOOST_FOREACH( chunk chk, chunks ) {
			std::cout << chk.id << " -> " << chk.dst << " (" << chk.score << ")" << std::endl;
			BOOST_FOREACH( token tok, chk.tokens ) {
				std::cout << "   " << tok.id << " " << tok.surf << " " << tok.orig << " " << tok.pos1 << " " << tok.fsem;
				if (tok.pas.is_pred() ) {
					std::cout << "\t" << tok.pas.pred_type << " ";
					boost::unordered_map< std::string, int >::iterator it;
					for (it=tok.pas.args.begin() ; it!=tok.pas.args.end() ; ++it) {
						std::cout << it->first << "=" << it->second << " ";
					}
				}
				std::cout << std::endl;
			}
		}
		return true;
	}


	void sentence::clear_mod() {
		std::vector<chunk>::iterator it_chk;
		std::vector<token>::iterator it_tok;
		for (it_chk = chunks.begin() ; it_chk != chunks.end() ; ++it_chk) {
			for (it_tok = it_chk->tokens.begin() ; it_tok != it_chk->tokens.end() ; ++it_tok) {
				it_tok->has_mod = false;
				it_tok->mod = modality();
			}
		}
	}

};




