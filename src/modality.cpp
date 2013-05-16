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
	/*
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
	*/


	bool parser::learnOC(std::vector< std::string > xmls) {
		BOOST_FOREACH ( std::string xml_path, xmls ) {
#ifdef DEBUG
			std::cout << xml_path << std::endl;
#endif
			
			std::vector< std::vector< t_token > > oc_sents = parse_OC(xml_path);
			std::vector< nlp::sentence > parsed_sents;
			BOOST_FOREACH ( std::vector< t_token > oc_sent, oc_sents ) {
				nlp::sentence mod_ipa_sent = make_tagged_ipasents( oc_sent );
			
				std::vector< nlp::chunk >::iterator it_chk;
				std::vector< nlp::token >::iterator it_tok;
				for (it_chk=mod_ipa_sent.chunks.begin() ; it_chk!=mod_ipa_sent.chunks.end() ; ++it_chk) {
					for (it_tok=it_chk->tokens.begin() ; it_tok!=it_chk->tokens.end() ; ++it_tok) {
						if (it_tok->has_mod && it_tok->mod.tag.find("actuality") != it_tok->mod.tag.end()) {
							std::cout << it_tok->mod.tag["actuality"];
							t_feat *feat;
							feat = new t_feat;
							gen_feature( mod_ipa_sent, it_tok->id, *feat );
							t_feat::iterator it_feat;
							for ( it_feat=feat->begin() ; it_feat!=feat->end() ; ++it_feat ) {
								std::cout << " " << it_feat->first << ":" << it_feat->second;
							}
							std::cout << std::endl;
						}
					}
				}
			}
		}
		
		return true;
	}


	nlp::sentence parser::make_tagged_ipasents( std::vector< t_token > sent_orig ) {
		nlp::sentence sent;
		sent.ma_tool = nlp::sentence::MeCab;
		
		std::string text = "";
		BOOST_FOREACH ( t_token tok, sent_orig ) {
			text += tok.orthToken;
		}
		
		std::string parsed_text = cabocha->parseToString( text.c_str() );
//		std::cout << parsed_text << std::endl;

		sent.parse_cabocha(parsed_text);
		
#ifdef DEBUG
		std::cout << std::endl;
#endif
		
#ifdef DEBUG
		std::vector<std::string> matchedIDs;
#endif
		int sp = 0, ep = 0;
		std::vector< nlp::chunk >::iterator it_chk;
		std::vector< nlp::token >::iterator it_tok;
		for (it_chk=sent.chunks.begin() ; it_chk!=sent.chunks.end() ; ++it_chk) {
			bool chk_has_mod = false;
			for (it_tok=it_chk->tokens.begin() ; it_tok!=it_chk->tokens.end() ; ++it_tok) {
				sp = ep;
				ep = sp + it_tok->surf.size() - 1;
#ifdef DEBUG
				std::cout << it_tok->surf << "(" << sp << "," << ep << ")" << std::endl;
#endif
				
				BOOST_FOREACH ( t_token tok, sent_orig ) {
					if (
							( tok.sp == sp && tok.ep == ep ) || 
							( tok.sp <= sp && ep <= tok.ep ) ||
							( sp <= tok.sp && tok.sp <= ep ) ||
							( sp <= tok.ep && tok.ep <= ep ) ||
							( sp <= tok.sp && tok.ep <= ep )
						 ) {
						
						if ( tok.eme.find("morphIDs") != tok.eme.end() ) {
							nlp::t_eme::iterator it_eme;
							for (it_eme=tok.eme.begin() ; it_eme!=tok.eme.end() ; ++it_eme) {
								it_tok->mod.tag[it_eme->first] = it_eme->second;
								it_tok->has_mod = true;
								chk_has_mod = true;
							}
#ifdef DEBUG
							std::cout << "found\t" << tok.orthToken << "(" << tok.morphID << ") - " << it_tok->surf << "(" << it_tok->id << ")" << std::endl;
							matchedIDs.push_back(tok.morphID);
#endif
						}
					}
				}
				ep++;
			}
			it_chk->has_mod = chk_has_mod;
		}
		
#ifdef DEBUG
		BOOST_FOREACH( t_token tok, sent_orig ) {
			if ( tok.eme.find("morphIDs") != tok.eme.end() && std::find(matchedIDs.begin(), matchedIDs.end(), tok.morphID) == matchedIDs.end() ) {
				std::cout << "not found\t" << tok.morphID << std::endl;
			}
		}
#endif
		
		return sent;
	}


	std::vector<t_token> parser::parse_OC_sent(tinyxml2::XMLElement *elemSent, int *sp) {
		std::vector<t_token> toks;
		
		if (std::string(elemSent->Name()) == "sentence") {
			tinyxml2::XMLElement *elem = elemSent->FirstChildElement("SUW");
			while (elem) {
				if (std::string(elem->Name()) == "SUW") {
					t_token tok;
					tok.orthToken = elem->Attribute("orthToken");
					tok.morphID = elem->Attribute("morphID");
					tok.sp = *sp;
					tok.ep = tok.sp + tok.orthToken.size() - 1;
					toks.push_back(tok);
					*sp = tok.ep + 1;
				}
				else if (std::string(elem->Name()) == "sentence") {
					BOOST_FOREACH(t_token tok, parse_OC_sent(elem, sp)) {
						toks.push_back(tok);
					}
				}
				elem = elem->NextSiblingElement();
			}
		}
		
		return toks;
	}


	std::vector< std::vector< t_token > > parser::parse_OC_sents (tinyxml2::XMLElement *elem) {
		std::vector< std::vector<t_token> > sents;
		
		tinyxml2::XMLElement *elemWL = elem->FirstChildElement("webLine");
		while (elemWL) {
			if (std::string(elemWL->Name()) == "webLine") {
				tinyxml2::XMLElement *elem = elemWL->FirstChildElement("sentence");
				while (elem) {
					if (std::string(elem->Name()) == "sentence") {
						int sp = 0;
						std::vector<t_token> toks = parse_OC_sent(elem, &sp);
						sents.push_back(toks);
					}
					elem = elem->NextSiblingElement();
				}
			}
			elemWL = elemWL->NextSiblingElement();
		}
		
		return sents;
	}


	bool parser::parse_OC_modtag(tinyxml2::XMLElement *elem, std::vector< std::vector< t_token > > *sents) {
		tinyxml2::XMLElement *elemWL = elem->FirstChildElement("webLine");
		boost::regex reg_or("OR");
		boost::smatch m;

		while (elemWL) {
			if (std::string(elemWL->Name()) == "webLine") {
				tinyxml2::XMLElement *elemSent = elemWL->FirstChildElement("sentence");
				while (elemSent) {
					tinyxml2::XMLElement *elemEME = elemSent->FirstChildElement("eme:event");
					while (elemEME) {
						if (std::string(elemEME->Name()) == "eme:event") {
							// event tag which has required tags, does not contain "OR" in morphIDs
							if (elemEME->Attribute("eme:orthTokens") &&
									elemEME->Attribute("eme:morphIDs") &&
									boost::regex_search(std::string(elemEME->Attribute("eme:morphIDs")), m, reg_or) == false )
							{
								bool use = true;
								if (elemEME->Attribute("eme:pseudo")) {
									std::string pseudo = elemEME->Attribute("eme:pseudo");
									if (pseudo == "機能語" || pseudo == "副詞" || pseudo == "連体詞" || pseudo == "名詞" || pseudo == "感嘆詞" || pseudo == "比況" || pseudo == "解析誤り") {
										use = false;
									}
									else if (pseudo == "名詞-事象可能") {
										use = false;
									}
								}

								if (use) {
									std::string orthToken = elemEME->Attribute("eme:orthTokens");
									std::vector<std::string> buf;
									boost::algorithm::split(buf, orthToken, boost::algorithm::is_any_of(","));
									orthToken = buf[0];

									std::string morphID = elemEME->Attribute("eme:morphIDs");
									boost::algorithm::split(buf, morphID, boost::algorithm::is_any_of(","));
									morphID = buf[buf.size()-1];

									std::vector< std::vector< t_token > >::iterator it_sent;
									std::vector< t_token >::iterator it_tok;
									for (it_sent=sents->begin() ; it_sent!=sents->end() ; ++it_sent) {
										for (it_tok=it_sent->begin() ; it_tok!=it_sent->end() ; ++it_tok) {
											if (it_tok->morphID == morphID) {
												const tinyxml2::XMLAttribute *attr = elemEME->FirstAttribute();
												while (attr) {
													std::string attr_name = attr->Name();
													boost::algorithm::split(buf, attr_name, boost::algorithm::is_any_of(":"));
													attr_name = buf[buf.size()-1];

													it_tok->eme[attr_name] = std::string(attr->Value());
													attr = attr->Next();
												}
											}
										}
									}
								}
							}
						}
						elemEME = elemEME->NextSiblingElement();
					}
					elemSent = elemSent->NextSiblingElement();
				}
			}
			elemWL = elemWL->NextSiblingElement();
		}
		
#ifdef DEBUG
		std::vector<std::string> attrs;
		attrs.push_back("source");
		attrs.push_back("time");
		attrs.push_back("conditional");
		attrs.push_back("pmtype");
		attrs.push_back("actuality");
		attrs.push_back("evaluation");
		BOOST_FOREACH( std::vector< t_token > toks, *sents ) {
			BOOST_FOREACH( t_token tok, toks ) {
				std::cout << tok.orthToken << " " << tok.morphID << " (" << tok.sp << "," << tok.ep << ")";
				boost::unordered_map< std::string, std::string >::iterator it;
				
				if (tok.eme.find("morphIDs") != tok.eme.end()) {
					std::cout << " " << tok.eme["morphIDs"];
					BOOST_FOREACH (std::string attr, attrs) {
						if (tok.eme.find(attr) != tok.eme.end()) {
							std::cout << " " << tok.eme[attr];
						}
					}
				}
				std::cout << std::endl;
			}
		}
#endif
		
		return true;
	}


	/*
	 * Convert modality-tagged OC xml into feature structure for ML
	 */
	std::vector< std::vector< t_token > > parser::parse_OC(std::string xml_path) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(xml_path.c_str());
		
		tinyxml2::XMLElement *elemQ = doc.FirstChildElement("sample")->FirstChildElement("OCQuestion");
		tinyxml2::XMLElement *elemA = doc.FirstChildElement("sample")->FirstChildElement("OCAnswer");
		std::vector< std::vector<t_token> > sentsQ;
		std::vector< std::vector<t_token> > sentsA;
		
		sentsQ = parse_OC_sents( elemQ );
		parse_OC_modtag( elemQ, &sentsQ);
		sentsA = parse_OC_sents( elemA );
		parse_OC_modtag( elemA, &sentsA);
		
		sentsQ.insert(sentsQ.end(), sentsA.begin(), sentsA.end());
	
		return sentsQ;
	}


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



