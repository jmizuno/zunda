#include <stdlib.h>
#include <iostream>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <iomanip>

#include "sentence.hpp"
#include "util.hpp"


std::string rm_quote(std::string str) {
	str.replace(0, 1, "");
	str.replace(str.size()-1, 1, "");

	return str;
}

namespace nlp {
	void modality::parse(std::string mod_line) {
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

			source = l[2];
			tense = l[3];
			assumptional = l[4];
			type = l[5];
			authenticity = l[6];
			sentiment = l[7];
			focus = l[8];
		}
	}

	std::string modality::str() {
		std::string str;
		std::string tid_str;
		join(tid_str, tids, ",");
		
		str = tid_str + "\t" + source + "\t" + tense + "\t" + assumptional + "\t" + type + "\t" + authenticity + "\t" + sentiment + "\t" + focus;

		return str;
	}
}


namespace nlp {
	bool pas::is_pred() {
		if (pred_type == "null") {
			return false;
		}
		else {
			return true;
		}
	}

	bool pas::parse(std::string pas_line, pas *pas) {
		std::vector<std::string> pas_infos;
		boost::algorithm::split(pas_infos, pas_line, boost::algorithm::is_any_of(" ") );

		BOOST_FOREACH(std::string pas_info, pas_infos) {
			std::vector<std::string> v;
			boost::algorithm::split(v, pas_info, boost::algorithm::is_any_of("=") );
			
			if (v.size() == 2) {
				if (v[0] == "type") {
					pas->pred_type = rm_quote(v[1]);
				}
				else if (v[0] == "ID") {
					pas->arg_id = boost::lexical_cast<int>(rm_quote(v[1]));
				}
				else {
					pas->arg_type[v[0]] = boost::lexical_cast<int>(rm_quote(v[1]));
				}
			}
				
		}
		
		return true;
	}
}

namespace nlp {
	bool token::parse_chasen(std::string line, int tok_id) {
		std::vector<std::string> tok_infos;
		boost::algorithm::split(tok_infos, line, boost::algorithm::is_any_of("\t") );

		id = tok_id;
		surf = tok_infos[0];
		read = tok_infos[1];
		orig = tok_infos[2];
		pos1 = tok_infos[3];
		pos2 = tok_infos[4];
		pos3 = tok_infos[5];

		std::vector<std::string> pos_infos;
		boost::algorithm::split(pos_infos, tok_infos[3], boost::is_any_of("-"));
		pos = pos_infos[0];
		
		if (tok_infos.size() > 6) {
			ne = tok_infos[6];
		}

		if (tok_infos.size() > 7) {
			pas.parse(tok_infos[7], &pas);
		}

		return true;
	}

	bool token::parse_mecab(std::string line, int tok_id) {
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
		
		if (tok_infos.size() > 2) {
			ne = tok_infos[2];
		}
		
		if (tok_infos.size() > 3) {
			pas.parse(tok_infos[3], &pas);
		}

		return true;
	}
}


namespace nlp {
	bool chunk::add_token(token _tok) {
		tokens.push_back(_tok);
		return true;
	}

	std::string chunk::str() {
		std::string chk_str = "";
		BOOST_FOREACH(token tok, tokens) {
			chk_str += tok.orig;
		}
		return chk_str;
	}

	token chunk::get_token_has_mod() {
		std::vector<nlp::token>::reverse_iterator rit_tok;
		for (rit_tok=tokens.rbegin() ; rit_tok!=tokens.rend() ; ++rit_tok) {
			if (rit_tok->has_mod) {
				return *rit_tok;
			}
		}
		token tok;
		return tok;
	}
	
};


namespace nlp {
	std::string sentence::str(std::string delimiter) {
		std::vector<std::string> chks;
		BOOST_FOREACH(chunk chk, chunks) {
			chks.push_back(chk.str());
		}
		
		std::string chk_str;
		join(chk_str, chks, delimiter);
		
		return chk_str;
	}

	std::string sentence::str() {
		return str("");
	}

	bool sentence::add_chunk(chunk chk) {
		chunks.push_back(chk);
		return true;
	}

	bool sentence::test(std::vector< std::string > lines) {
		BOOST_FOREACH(std::string l, lines) {
			std::cout << l << std::endl;
		}
		return true;
	}
	
	bool sentence::parse_cabocha(std::string str) {
		std::vector< std::string > buf;
		boost::algorithm::split(buf, str, boost::algorithm::is_any_of("\n") );
		parse_cabocha(buf);
		return true;
	}

	bool sentence::parse_cabocha(std::vector< std::string > lines) {
		boost::smatch m;
		int tok_cnt = 0;

		std::vector< modality > mods;

		BOOST_FOREACH( std::string l, lines ) {
			if (l.compare(0, 6, "#EVENT") == 0 ) {
				modality mod;
				mod.parse(l);
				mods.push_back(mod);
			}
			else if (l.compare(0, 1, "#") == 0) {
			}
			else if (boost::regex_search(l, m, chk_line)) {
				chunk chk;
				std::vector<std::string> v, v2;
				boost::algorithm::split(v, l, boost::algorithm::is_space() );
				chk.id = boost::lexical_cast<int>(v[1]);

				if (chk.id != (int)chunks.size()) {
					std::cerr << "error: chunk id is not in order" << std::endl;
					return false;
				}
				
				std::string dst_str = boost::regex_replace(v[2], chk_dst, dst_rep);
				std::string type_str = boost::regex_replace(v[2], chk_dst, type_rep);
				chk.dst = boost::lexical_cast<int>(dst_str);
				chk.type = boost::lexical_cast<std::string>(type_str);

				if (v.size() > 3) {
					boost::algorithm::split(v2, v[3], boost::is_any_of("/"));
					chk.subj = boost::lexical_cast<int>(v2[0]);
					chk.func = boost::lexical_cast<int>(v2[1]);

					chk.score = boost::lexical_cast<double>(v[4]);
				}

				chunks.push_back(chk);
			}
			else if (l.compare(0, 3, "EOS") == 0) {
				break;
			}
			else {
				token tok;
				
				switch(ma_tool) {
					case ChaSen:
						tok.parse_chasen(l, tok_cnt);
						break;
					case MeCab:
						tok.parse_mecab(l, tok_cnt);
						break;
					default:
						std::cerr << "Error: unknown Morphological Analysis tool" << std::endl;
						exit(1);
						break;
				}
				
// ToDo parse v[7] as pas

				t2c[tok.id] = chunks[chunks.size()-1].id;
				chunks[chunks.size()-1].add_token(tok);
				
				tok_cnt++;
			}
		}

		tid_min = 0;
		tid_max = tok_cnt-1;
		cid_min = 0;
		cid_max = chunks.size()-1;

		std::vector<chunk>::iterator it_chk;
		std::vector<token>::iterator it_tok;
		for (it_chk = chunks.begin() ; it_chk != chunks.end() ; ++it_chk) {
			for (it_tok = it_chk->tokens.begin() ; it_tok != it_chk->tokens.end() ; ++it_tok) {
				BOOST_FOREACH (modality mod, mods) {
					if ( find(mod.tids.begin(), mod.tids.end(), it_tok->id) != mod.tids.end() ) {
						it_tok->mod = mod;
						it_tok->has_mod = true;
						it_chk->has_mod = true;
					}

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
		}
		
		return true;
	}

	chunk sentence::get_chunk(int id) {
		if (cid_min <= id && id <= cid_max) {
			return chunks[id];
		}
		else {
			std::cerr << "error: invalid chunkd id" << std::endl;
			exit(-1);
		}
	}

	chunk sentence::get_chunk_by_tokenID(int id) {
		int cid = t2c[id];
		return get_chunk(cid);
	}


	token sentence::get_token(int id) {
		int cid = t2c[id];
		chunk chk = get_chunk(cid);
		if (tid_min <= id && id <= tid_max) {
			BOOST_FOREACH(token tok, chk.tokens) {
				if (tok.id == id) {
					return tok;
				}
			}
			std::cerr << "ERROR: invalid token id" << std::endl;
			exit(-1);
		}
		else {
			std::cerr << "error: invalid token id" << std::endl;
			exit(-1);
		}
	}


	bool sentence::get_chunk_has_mod(chunk &chk, int cid) {
		chunk chk_core = get_chunk(cid);
		if (chk_core.dst == -1) {
			return false;
		}
		
		chk = get_chunk(chk_core.dst);
		while (chk.has_mod == false) {
			if (chk.dst == -1) {
				return false;
			}
			chk = get_chunk(chk.dst);
		}
		
		return true;
	}


	std::string sentence::cabocha() {
		std::stringstream cabocha_ss;
		int eve_id = 0;
		BOOST_FOREACH ( chunk chk, chunks ) {
			BOOST_FOREACH ( token tok, chk.tokens) {
				if (tok.has_mod) {
					cabocha_ss << "#EVENT" << eve_id << "\t" << tok.mod.str() << "\n";
					eve_id++;
				}
			}
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
		
		return cabocha_ss.str();
	}


	bool sentence::pp() {
		std::cout << sent_id << std::endl;
		BOOST_FOREACH( chunk chk, chunks ) {
			std::cout << chk.id << " -> " << chk.dst << " (" << chk.score << ")" << std::endl;
			BOOST_FOREACH( token tok, chk.tokens ) {
				std::cout << "   " << tok.id << " " << tok.surf << " " << tok.orig << " " << tok.pos1;
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
};




