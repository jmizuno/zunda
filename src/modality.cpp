#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
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


	void parser::read_model(model_type& model, classias::quark& labels, std::istream& is) {
		while(true) {
			std::string line;
			std::getline(is, line);
			if (is.eof()) {
				break;
			}
			
			if (line.compare(0, 7, "@label\t") == 0) {
				labels(line.substr(7));
				continue;
			}

			if (line.compare(0, 1, "@") == 0) {
				continue;
			}

			unsigned int pos = line.find('\t');
			if (pos == line.npos) {
				std::cerr << "feature weight is missing" << std::endl;
				exit(-1);
			}

			double w = std::atof(line.c_str());
			if (++pos == line.size()) {
				std::cerr << "feature name is missing" << std::endl;
				exit(-1);
			}

			model.insert(model_type::pair_type(line.substr(pos), w));
		}
	}


	nlp::sentence parser::classify(model_type model, classias::quark labels, std::string text, int input_layer) {

		nlp::sentence sent;
		sent.ma_tool = nlp::sentence::MeCab;
		std::string parsed_text;
		switch (input_layer) {
			case raw_text:
				parsed_text = cabocha->parseToString( text.c_str() );
				break;
			case cabocha_text:
				parsed_text = text;
				break;
			case chapas_text:
				parsed_text = text;
				break;
			default:
				std::cerr << "invalid input format" << std::endl;
				break;
		}
		sent.parse_cabocha(parsed_text);
		
		sent = classify(model, labels, sent);
		
		return sent;
	}


	nlp::sentence parser::classify(model_type model, classias::quark labels, nlp::sentence sent) {
		std::vector<nlp::chunk>::reverse_iterator rit_chk;
		std::vector<nlp::token>::reverse_iterator rit_tok;
		for (rit_chk=sent.chunks.rbegin() ; rit_chk!=sent.chunks.rend() ; ++rit_chk) {
			for (rit_tok=(rit_chk->tokens).rbegin() ; rit_tok!=(rit_chk->tokens).rend() ; ++rit_tok) {
				if (
						(pred_detect_rule && (rit_tok->pos == "動詞" || rit_tok->pos == "形容詞" || (rit_tok->pos == "名詞" && rit_tok->pos1 == "サ変接続") ))
						||
						(!pred_detect_rule && rit_tok->pas.is_pred())
					 ) {
					classifier_type inst(model);
					inst.clear();
					inst.resize(labels.size());
					feature_generator fgen;

					t_feat *feat;
					feat = new t_feat;
					gen_feature( sent, rit_tok->id, *feat );

#ifdef _MODEBUG
					std::string feat_str = "";
#endif
					t_feat::iterator it_feat;
					for (it_feat=feat->begin() ; it_feat!=feat->end() ; ++it_feat) {
#ifdef _MODEBUG
						std::stringstream ss;
						ss << it_feat->first;
						ss << ":";
						ss << it_feat->second;
						ss << " ";
						feat_str += ss.str();
#endif

						for (int i=0 ; i<(int)labels.size() ; ++i) {
							inst.set(i, fgen, it_feat->first, labels.to_item(i), it_feat->second);
						}
					}
					inst.finalize();

#ifdef _MODEBUG
					std::cout << feat_str << std::endl;
					std::cout << " -> " << labels.to_item(inst.argmax()) << std::endl;
#endif

					rit_tok->mod.tids.push_back(rit_tok->id);
					rit_tok->mod.authenticity = labels.to_item(inst.argmax());
					rit_tok->has_mod = true;
					rit_chk->has_mod = true;
				}
			}
		}
		return sent;
	}

	
	void parser::load_deppasmods(std::vector< std::string > deppasmods) {
		learning_data.clear();
		
		BOOST_FOREACH ( std::string deppasmod, deppasmods ) {
			std::cout << deppasmod << std::endl;
			boost::filesystem::path p(deppasmod);
			std::string sent_id = p.stem().string();

			nlp::sentence sent;
			sent.sent_id = sent_id;
			sent.ma_tool = sent.MeCab;
			std::ifstream ifs(deppasmod.c_str());
			std::string buf, str;
			std::vector< std::string > lines;
			while ( getline(ifs, buf) ) {
				lines.push_back(buf);
			}
			sent.parse_cabocha(lines);
			learning_data.push_back(sent);
		}
	}
	

	void parser::load_xmls(std::vector< std::string > xmls) {
		learning_data.clear();

		BOOST_FOREACH ( std::string xml_path, xmls ) {
			std::cout << xml_path << std::endl;
			boost::filesystem::path p(xml_path);
			std::string doc_id = p.stem().string();
			
			std::vector< std::vector< t_token > > oc_sents = parse_OC(xml_path);
			std::vector< nlp::sentence > parsed_sents;
			unsigned int sent_cnt = 0;
			BOOST_FOREACH ( std::vector< t_token > oc_sent, oc_sents ) {
				nlp::sentence mod_ipa_sent = make_tagged_ipasents( oc_sent );
				mod_ipa_sent.doc_id = doc_id;
				std::stringstream sent_id;
				sent_id << doc_id << "_" << std::setw(3) << std::setfill('0') << sent_cnt;
				sent_cnt++;
				mod_ipa_sent.sent_id = sent_id.str();
				learning_data.push_back(mod_ipa_sent);
			}
		}
	}
	

	bool parser::learnOC(std::string model_path, std::string feature_path) {
		classias::msdata data;
		
		std::ofstream os_feat(feature_path.c_str());
		
		BOOST_FOREACH ( nlp::sentence sent, learning_data ) {
			std::vector< nlp::chunk >::iterator it_chk;
			std::vector< nlp::token >::iterator it_tok;
			for (it_chk=sent.chunks.begin() ; it_chk!=sent.chunks.end() ; ++it_chk) {
				for (it_tok=it_chk->tokens.begin() ; it_tok!=it_chk->tokens.end() ; ++it_tok) {
					if (it_tok->has_mod && it_tok->mod.authenticity != "") {
						classias::minstance& inst = data.new_element();

						std::string label = it_tok->mod.authenticity;
						inst.set_group(0);
						inst.set_label(data.labels(label));

						t_feat *feat;
						feat = new t_feat;
						gen_feature( sent, it_tok->id, *feat );
						t_feat::iterator it_feat;
						os_feat << label;
						for (it_feat=feat->begin() ; it_feat!=feat->end() ; ++it_feat) {
							os_feat << " " << it_feat->first << ":" << it_feat->second;
							inst.append(data.attributes(it_feat->first), it_feat->second);
						}
						os_feat << std::endl;
					}
				}
			}
		}
		os_feat.close();

		if (data.empty()) {
			std::cerr << "The data set is empty" << std::endl;
			exit(-1);
		}

		data.generate_features();
		std::cerr << "Number of instances: " << data.size() << std::endl;
		std::cerr << "Number of attributes: " << data.num_attributes() << std::endl;
		std::cerr << "Number of labels: " << data.num_labels() << std::endl;
		std::cerr << "Number of features: " << data.num_features() << std::endl;

		trainer_type tr;
		//tr.params().set("max_iterations", 100);
		tr.train(data, std::cerr);

		std::ofstream os_model(model_path.c_str());
		os_model << "@classias\tlinear\tmulti\t";
		os_model << data.feature_generator.name() << std::endl;

		for (int l = 0;l < data.num_labels();++l) {
			os_model << "@label\t" << data.labels.to_item(l) << std::endl;
		}

		for (int i = 0;i < data.num_features();++i) {
			double w = tr.model()[i];
			if (w != 0.) {
				int a, l;
				data.feature_generator.backward(i, a, l);
				const std::string& attr = data.attributes.to_item(a);
				const std::string& label = data.labels.to_item(l);
				os_model << w << '\t' << attr << '\t' << label << std::endl;
			}
		}
		os_model.close();
		
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
		
#ifdef _MODEBUG
		std::cout << std::endl;
#endif

#ifdef _MODEBUG
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
#ifdef _MODEBUG
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
								if (it_eme->first == "source") {
									it_tok->mod.source = it_eme->second;
								}
								else if (it_eme->first == "time") {
									it_tok->mod.tense = it_eme->second;
								}
								else if (it_eme->first == "conditional") {
									it_tok->mod.assumptional = it_eme->second;
								}
								else if (it_eme->first == "pmtype") {
									it_tok->mod.type = it_eme->second;
								}
								else if (it_eme->first == "actuality") {
									it_tok->mod.authenticity = it_eme->second;
								}
								else if (it_eme->first == "evaluation") {
									it_tok->mod.sentiment = it_eme->second;
								}
								else if (it_eme->first == "focus") {
									it_tok->mod.focus = it_eme->second;
								}
							}

							it_tok->mod.tids.push_back(it_tok->id);
							it_tok->has_mod = true;
							chk_has_mod = true;
#ifdef _MODEBUG
							std::cout << "found\t" << tok.orthToken << "(" << tok.morphID << ") - " << it_tok->surf << "(" << it_tok->id << ")" << std::endl;
							matchedIDs.push_back(tok.morphID);
#endif
						}
					}
				}
				ep++;
				sort(it_tok->mod.tids.begin(), it_tok->mod.tids.end());
				it_tok->mod.tids.erase(unique(it_tok->mod.tids.begin(), it_tok->mod.tids.end()), it_tok->mod.tids.end());
			}
			it_chk->has_mod = chk_has_mod;
		}
		
#ifdef _MODEBUG
		BOOST_FOREACH( t_token tok, sent_orig ) {
			if ( tok.eme.find("morphIDs") != tok.eme.end() && std::find(matchedIDs.begin(), matchedIDs.end(), tok.morphID) == matchedIDs.end() ) {
				std::cout << "not found\t" << tok.morphID << std::endl;
			}
		}
#endif
		
		return sent;
	}


	std::vector<t_token> parser::parse_bccwj_sent(tinyxml2::XMLElement *elemSent, int *sp) {
		std::vector<t_token> toks;
		
		if ( std::string(elemSent->Name()) == "sentence" || std::string(elemSent->Name()) == "quote" ) {
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
				else if ( std::string(elem->Name()) == "sentence" || std::string(elem->Name()) == "quote" ) {
					BOOST_FOREACH(t_token tok, parse_bccwj_sent(elem, sp)) {
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
						std::vector<t_token> sent = parse_bccwj_sent(elem, &sp);
						parse_modtag_for_sent(elem, &sent);
						sents.push_back(sent);
					}
					elem = elem->NextSiblingElement();
				}
			}
			elemWL = elemWL->NextSiblingElement();
		}
		
		return sents;
	}


	void parser::parse_modtag_for_sent(tinyxml2::XMLElement *elemSent, std::vector< t_token > *sent) {
		boost::regex reg_or("OR");
		boost::smatch m;

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

							std::vector< t_token >::iterator it_tok;
							for (it_tok=sent->begin() ; it_tok!=sent->end() ; ++it_tok) {
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
				elemEME = elemEME->NextSiblingElement();
			}
			elemSent = elemSent->NextSiblingElement();
		}

#ifdef _MODEBUG
		std::vector<std::string> attrs;
		attrs.push_back("source");
		attrs.push_back("time");
		attrs.push_back("conditional");
		attrs.push_back("pmtype");
		attrs.push_back("actuality");
		attrs.push_back("evaluation");
		BOOST_FOREACH( t_token tok, sent ) {
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
#endif
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
//		parse_OC_modtag( elemQ, &sentsQ);
		sentsA = parse_OC_sents( elemA );
//		parse_OC_modtag( elemA, &sentsA);
		
		sentsQ.insert(sentsQ.end(), sentsA.begin(), sentsA.end());
	
		return sentsQ;
	}


	std::vector< std::vector< t_token > > parser::parse_OW_PB_PN(std::string xml_path) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(xml_path.c_str());
		
		tinyxml2::XMLElement *elem = doc.FirstChildElement("mergedSample");
		tinyxml2::XMLElement *elemSent = elem->FirstChildElement("sentence");
		std::vector< std::vector<t_token> > sents;
		while (elemSent) {
			if (std::string(elemSent->Name()) == "sentence") {
				int sp = 0;
				std::vector<t_token> sent = parse_bccwj_sent(elemSent, &sp);
				parse_modtag_for_sent(elemSent, &sent);
				sents.push_back(sent);
			}
			elemSent = elemSent->NextSiblingElement();
		}
		
		return sents;
	}
};



