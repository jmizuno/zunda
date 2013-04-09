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
					t_feat *feat;
					feat = new t_feat;

					gen_feature(sent, rit_tok->id, *feat);
					t_feat::iterator it_feat;
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
		t_feat *feat;
		feat = new t_feat;
		
		conv_instance_OC(xml_path, *feat);
		
		return true;
	}

	

	std::vector<t_token> parser::parseOCXML_sent(tinyxml2::XMLElement *elemSent, int *sp) {
		std::vector<t_token> toks;
		
		if (std::string(elemSent->Name()) == "sentence") {
			tinyxml2::XMLElement *elem = elemSent->FirstChildElement("SUW");
			while (elem) {
				if (std::string(elem->Name()) == "SUW") {
					t_token tok;
					tok.orthToken = elem->Attribute("orthToken");
					tok.morphID = elem->Attribute("morphID");
					tok.sp = *sp;
					tok.ep = tok.sp + tok.orthToken.size();
					toks.push_back(tok);
					*sp = tok.ep + 1;
				}
				else if (std::string(elem->Name()) == "sentence") {
					BOOST_FOREACH(t_token tok, parseOCXML_sent(elem, sp)) {
						toks.push_back(tok);
					}
				}
				elem = elem->NextSiblingElement();
			}
		}
		
		return toks;
	}


	
	/*
	 * Convert modality-tagged OC xml into feature structure for ML
	 */
	bool parser::conv_instance_OC(std::string xml_path, t_feat &feat) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(xml_path.c_str());
		
//		std::vector<t_mod_tag> mod_tags;
		
		tinyxml2::XMLElement *elemQ = doc.FirstChildElement("sample")->FirstChildElement("OCQuestion");
		tinyxml2::XMLElement *elemA = doc.FirstChildElement("sample")->FirstChildElement("OCAnswer");
		std::vector< std::vector<t_token> > sents;
		
		tinyxml2::XMLElement *elemWL = elemQ->FirstChildElement("webLine");
		while (elemWL) {
			if (std::string(elemWL->Name()) == "webLine") {
				tinyxml2::XMLElement *elem = elemWL->FirstChildElement("sentence");
				while (elem) {
					if (std::string(elem->Name()) == "sentence") {
						int sp = 0;
						std::vector<t_token> toks = parseOCXML_sent(elem, &sp);
						sents.push_back(toks);
					}
					elem = elem->NextSiblingElement();
				}
			}
			elemWL = elemWL->NextSiblingElement();
		}
		
		elemWL = elemQ->FirstChildElement("webLine");
		while (elemWL) {
			if (std::string(elemWL->Name()) == "webLine") {
				tinyxml2::XMLElement *elemEME = elemWL->FirstChildElement("sentence")->FirstChildElement("eme:event");
				while (elemEME) {
					if (std::string(elemEME->Name()) == "eme:event") {
						if (elemEME->Attribute("eme:orthTokens") != NULL && elemEME->Attribute("eme:morphIDs") != NULL) {
							std::string orthToken = elemEME->Attribute("eme:orthTokens");
							std::vector<std::string> buf;
							boost::algorithm::split(buf, orthToken, boost::algorithm::is_any_of(","));
							orthToken = buf[0];

							std::string morphID = elemEME->Attribute("eme:morphIDs");
							boost::algorithm::split(buf, morphID, boost::algorithm::is_any_of(","));
							morphID = buf[0];

							bool found = false;
							BOOST_FOREACH(std::vector<t_token> toks, sents) {
								BOOST_FOREACH(t_token tok, toks) {
									if (tok.morphID == morphID) {
										std::cout << elemEME->Attribute("eme:morphIDs") << " " << elemEME->Attribute("eme:orthTokens") << " found at " << tok.sp << "-" << tok.ep << std::endl;
										found = true;
									}
								}
							}
							if (found == false) {
								std::cout << elemEME->Attribute("eme:morphIDs") << " " << elemEME->Attribute("eme:orthTokens") << " not found" << std::endl;
							}
						}
					}
					elemEME = elemEME->NextSiblingElement();
				}
			}
			elemWL = elemWL->NextSiblingElement();
		}

		
		BOOST_FOREACH (std::vector<t_token> toks, sents) {
			BOOST_FOREACH(t_token tok, toks) {
				std::cout << tok.sp << " - " << tok.ep << " " << tok.morphID << " " << tok.orthToken << std::endl;
			}
			std::cout << std::endl;
		}
		
		return true;
	}


#if 0

		tinyxml2::XMLElement *next_elem = doc.FirstChildElement("sample")->FirstChildElement("OCQuestion")->FirstChildElement("webLine");
		while (next_elem) {
			int sp = 0;
			int ep = 0;
			if (std::string(next_elem->Name()) == "webLine") {
//				std::cout << next_elem->FirstChildElement("sentence")->Name() << std::endl;
				
				std::stringstream split_tokens;
				tinyxml2::XMLElement *elem = next_elem->FirstChildElement("sentence")->FirstChildElement();
//				std::cout << elem->Name() << std::endl;
				while (elem) {
					if (std::string(elem->Name()) == "SUW") {
						std::string orthToken = elem->Attribute("orthToken");
						
//						split_tokens << orthToken << "\t*\n";
						t_mod_tag mod_tag;
						int token_len = orthToken.size();
						mod_tag.sp = sp;
						ep = sp + token_len;
						mod_tag.ep = ep;
						mod_tag.orthToken = orthToken;
						mod_tag.morphID = elem->Attribute("morphID");
						mod_tags.push_back(mod_tag);
//						std::cout << mod_tag.sp << "\t" << mod_tag.ep << "\t" << mod_tag.orthToken << std::endl;

						sp = ep + 1;
					}
					else if (std::string(elem->Name()) == "eme:event") {
						//std::cout << elem->Attribute("eme:morphIDs") << std::endl;
						std::string orthToken = elem->Attribute("eme:orthTokens");
						std::vector<std::string> buf;
						boost::algorithm::split(buf, orthToken, boost::algorithm::is_any_of(","));
						orthToken = buf[0];
						
						bool found = false;
						BOOST_FOREACH(t_mod_tag mod_tag, mod_tags) {
							if (mod_tag.morphID == elem->Attribute("eme:morphIDs") && mod_tag.orthToken == orthToken) {
								std::cout << elem->Attribute("eme:morphIDs") << " " << elem->Attribute("eme:orthTokens") << " found at " << mod_tag.sp << "-" << mod_tag.ep << std::endl;
								found = true;
							}
						}
						if (found == false) {
							std::cout << elem->Attribute("eme:morphIDs") << " " << elem->Attribute("eme:orthTokens") << " not found" << std::endl;
						}
					}
					elem = elem->NextSiblingElement();
					if (elem == NULL) {
						break;
					}
				}
				/*
				std::string tagged = mecab->parse(split_tokens.str().c_str());
				std::cout << tagged << std::endl;
				std::string parsed = cabocha->parseToString(tagged.c_str());
				std::cout << parsed << std::endl;

				nlp::sentence sent;
				sent.ma_tool = sent.MeCab;
				std::vector<std::string> lines;
				boost::algorithm::split(lines, parsed, boost::algorithm::is_any_of("\n"));
				sent.parse_cabocha(lines);
				sent.pp();
				*/

			}
			next_elem = next_elem->NextSiblingElement();
		}
		
		return true;
	}
#endif

	bool parser::gen_feature(nlp::sentence sent, int tok_id, t_feat &feat) {
		gen_feature_basic(sent, tok_id, feat, 3);

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



