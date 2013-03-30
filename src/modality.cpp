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

#include "sentence.hpp"
#include "modality.hpp"


namespace modality {
	bool parser::parse(std::string str) {
		std::vector<std::string> lines;
		boost::algorithm::split(lines, str, boost::algorithm::is_any_of("\n"));
		BOOST_FOREACH(std::string line, lines) {
			std::cout << line << std::endl;
		}

		nlp::sentence sent;
		sent.ma_tool = sent.ChaSen;
		sent.parse_cabocha(lines);
		sent.pp();

		std::vector<nlp::chunk>::reverse_iterator rit_chk;
		std::vector<nlp::token>::reverse_iterator rit_tok;
		for (rit_chk=sent.chunks.rbegin() ; rit_chk!=sent.chunks.rend() ; ++rit_chk) {
			std::cout << rit_chk->id << std::endl;
			for (rit_tok=(rit_chk->tokens).rbegin() ; rit_tok!=(rit_chk->tokens).rend() ; ++rit_tok) {
				if ((rit_tok->pas).is_pred()) {
					instance::t_feat *feat;
					feat = new instance::t_feat;

					gen_feature(sent, rit_tok->id, *feat);
					instance::t_feat::iterator it_feat;
					std::cout << "feature " << rit_tok->id << std::endl;
					for (it_feat=feat->begin() ; it_feat!=feat->end() ; ++it_feat) {
						std::cout << "\t" << it_feat->first << " : " << it_feat->second << std::endl;
					}
				}
			}
		}

		return true;
	}

	bool parser::learnOC(std::string xml_path) {
		instance::t_feat *feat;
		feat = new instance::t_feat;
		
		conv_instance_OC(xml_path, *feat);
		
		return true;
	}

	bool parser::conv_instance_OC(std::string xml_path, instance::t_feat &feat) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(xml_path.c_str());
		
		tinyxml2::XMLElement *next_elem = doc.FirstChildElement("sample")->FirstChildElement("OCQuestion")->FirstChildElement("webLine");
		while (next_elem) {
			if (std::string(next_elem->Name()) == "webLine") {
				std::cout << next_elem->FirstChildElement("sentence")->Name() << std::endl;
				
				tinyxml2::XMLElement *elem = next_elem->FirstChildElement("sentence")->FirstChildElement();
				std::cout << elem->Name() << std::endl;
				while (elem) {
					if (std::string(elem->Name()) == "SUW") {
						std::cout << elem->Attribute("orthToken") << std::endl;
					}
					else if (std::string(elem->Name()) == "eme:event") {
						std::cout << elem->Attribute("eme:morphIDs") << std::endl;
					}
					elem = elem->NextSiblingElement();
					if (elem == NULL) {
						break;
					}
				}
			}
			next_elem = next_elem->NextSiblingElement();
		}
		
		return true;
	}

	bool parser::gen_feature(nlp::sentence sent, int tok_id, instance::t_feat &feat) {
		gen_feature_basic(sent, tok_id, feat, 3);

		return true;
	}

	bool parser::gen_feature_basic(nlp::sentence sent, int tok_id, instance::t_feat &feat, int n) {
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



