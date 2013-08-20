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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <tinyxml2.h>
#include <mecab.h>
#include <cabocha.h>
#include <iomanip>

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


	std::string parser::id2tag(unsigned int id) {
		switch(id) {
			case SOURCE:
				return "source";
			case TENSE:
				return "tense";
			case ASSUMPTIONAL:
				return "assumptional";
			case TYPE:
				return "type";
			case AUTHENTICITY:
				return "authenticity";
			case SENTIMENT:
				return "sentiment";
			default:
				return NULL;
		}
	}


	void parser::set_model_dir(std::string dir) {
		boost::filesystem::path dir_path(dir);
		set_model_dir(dir_path);
	}


	void parser::set_model_dir(boost::filesystem::path dir_path) {
		for (unsigned int i=0 ; i<LABEL_NUM ; ++i) {
			boost::filesystem::path mp("model_" + id2tag(i));
			model_path[i] = dir_path / mp;

			boost::filesystem::path fp("feat_" + id2tag(i));
			feat_path[i] = dir_path / fp;
		}
		
		boost::filesystem::path f2ip("feat2id.kch");
		f2i_path = dir_path / f2ip;
		boost::filesystem::path l2ip("label2id.kch");
		l2i_path = dir_path / l2ip;
	}


	bool parser::load_models(boost::filesystem::path *model_path_new) {
		model_path = model_path_new;
		return load_models();
	}


	bool parser::load_models() {
		boost::system::error_code error;
		BOOST_FOREACH (unsigned int i, analyze_tags) {
			const bool res = boost::filesystem::exists(model_path[i], error);
			if (!res || error) {
				std::cerr << "ERROR: " << model_path[i].string() << " not found" << std::endl;
				return false;
			}
			else {
#ifdef _MODEBUG
				std::cerr << "loaded " << model_path[i].string() << ": " << boost::filesystem::file_size(model_path[i]) << " byte, " << boost::posix_time::from_time_t(boost::filesystem::last_write_time(model_path[i])) << std::endl;
#endif
				models[i] = linear::load_model( model_path[i].string().c_str() );
			}
		}

		return true;
	}


	nlp::sentence parser::analyze(std::string text, int input_layer) {
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

		nlp::sentence parsed_sent = analyze(sent);

		return parsed_sent;
	}


	bool comp_xx(const linear::feature_node &xx1, const linear::feature_node &xx2) {
		return xx1.index < xx2.index;
	}

	linear::feature_node* parser::pack_feat_linear(t_feat *feat, bool read_only) {
		linear::feature_node* xx = new linear::feature_node[feat->size()+1];
		t_feat::iterator it_feat;
		int feat_cnt = 0;
		for (it_feat=feat->begin() ; it_feat!=feat->end() ; ++it_feat) {
			int feat_id = -1;
			if (read_only) {
				std::string val;
				if (f2iDB.get(it_feat->first, &val)) {
					feat_id = boost::lexical_cast<int>(val);
				}
			}
			else {
				if (feat2id.find(it_feat->first) == feat2id.end()) {
					feat2id[it_feat->first] = feat2id.size();
				}
				feat_id = feat2id[it_feat->first];
			}
			
			if (feat_id != -1) {
				xx[feat_cnt].index = feat_id;
				xx[feat_cnt].value = it_feat->second;
				feat_cnt++;
			}
		}
		// sorted by feature ID
		std::sort(xx, xx+feat_cnt-1, comp_xx);
		xx[feat_cnt].index = -1;
		
		return xx;
	}


	nlp::sentence parser::analyze(nlp::sentence sent) {
		std::vector<nlp::chunk>::reverse_iterator rit_chk;
		std::vector<nlp::token>::reverse_iterator rit_tok;
		for (rit_chk=sent.chunks.rbegin() ; rit_chk!=sent.chunks.rend() ; ++rit_chk) {
			for (rit_tok=(rit_chk->tokens).rbegin() ; rit_tok!=(rit_chk->tokens).rend() ; ++rit_tok) {
				if (
						(pred_detect_rule && ( (rit_tok->pos == "動詞" && rit_tok->pos1 == "自立") || (rit_tok->pos == "形容詞" && rit_tok->pos1 == "自立") || (rit_tok->pos == "名詞" && rit_tok->pos1 == "サ変接続") || (rit_tok->pos == "名詞" && rit_tok->pos1 == "形容動詞語幹") ))
						||
						(!pred_detect_rule && rit_tok->pas.is_pred())
					 ) {
					
					t_feat *feat;
					feat = new t_feat;
					gen_feature( sent, rit_tok->id, *feat );
					t_feat::iterator it_feat;

					linear::feature_node* xx = pack_feat_linear(feat, true);

#ifdef _MODEBUG
					std::string feat_str = "";
					for (it_feat=feat->begin() ; it_feat!=feat->end() ; ++it_feat) {
						std::stringstream ss;
						ss << it_feat->first;
						ss << ":";
						ss << it_feat->second;
						ss << " ";
						feat_str += ss.str();
					}

					std::cout << feat_str << std::endl;
#endif

#ifdef _MODEBUG
					std::cout << " ->";
#endif
					std::string labels[LABEL_NUM];
					BOOST_FOREACH (unsigned int i, analyze_tags) {
						int predicted = linear::predict(models[i], xx);
						boost::unordered_map< std::string, int >::iterator it;
						for (it=label2id.begin() ; it!=label2id.end() ; ++it) {
							if (predicted == it->second) {
								labels[i] = it->first;
								break;
							}
						}
						if (labels[i] == "") {
							std::cerr << "ERORR: unknown predicted label: " << predicted << std::endl;
							exit(-1);
						}
#ifdef _MODEBUG
						std::cout << " " << labels[i] << "(" << predicted << ")";
#endif
					}
#ifdef _MODEBUG
					std::cout << std::endl;
#endif

					rit_tok->mod.tids.push_back(rit_tok->id);
					BOOST_FOREACH (unsigned int i, analyze_tags) {
						rit_tok->mod.tag[id2tag(i)] = labels[i];
					}
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


	void parser::learn(boost::filesystem::path *model_path_new, boost::filesystem::path *feat_path_new) {
		model_path = model_path_new;
		feat_path = feat_path_new;
		learn();
	}


	void parser::learn() {
		int node_cnt = 0;
		BOOST_FOREACH (nlp::sentence sent, learning_data) {
			BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
				BOOST_FOREACH (nlp::token tok, chk.tokens) {
					if (
							(pred_detect_rule && tok.has_mod && ( (tok.pos == "動詞" && tok.pos1 == "自立") || (tok.pos == "形容詞" && tok.pos1 == "自立") || (tok.pos == "名詞" && tok.pos1 == "サ変接続") || (tok.pos == "名詞" && tok.pos1 == "形容動詞語幹") ))
							||
							(!pred_detect_rule && tok.has_mod && tok.mod.tag["authenticity"] != "")
						 ) {
						node_cnt++;
					}
				}
			}
		}

		linear::feature_node **x = new linear::feature_node*[node_cnt+1];
		int y[LABEL_NUM][node_cnt+1];

		node_cnt = 0;
		BOOST_FOREACH (nlp::sentence sent, learning_data) {
			BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
				BOOST_FOREACH (nlp::token tok, chk.tokens) {
					if (
							(pred_detect_rule && tok.has_mod && ( (tok.pos == "動詞" && tok.pos1 == "自立") || (tok.pos == "形容詞" && tok.pos1 == "自立") || (tok.pos == "名詞" && tok.pos1 == "サ変接続") || (tok.pos == "名詞" && tok.pos1 == "形容動詞語幹") ))
							||
							(!pred_detect_rule && tok.has_mod && tok.mod.tag["authenticity"] != "")
						 ) {
						BOOST_FOREACH (unsigned int i, analyze_tags) {
							std::string label = tok.mod.tag[id2tag(i)];
							if (label2id.find(label) == label2id.end()) {
								label2id[label] = label2id.size();
							}
							y[i][node_cnt] = label2id[label];
//							os_feat << label << "(" << label2id[label] << ") ";
						}

						t_feat *feat;
						feat = new t_feat;
						gen_feature(sent, tok.id, *feat);
						t_feat::iterator it_feat;
						
						linear::feature_node* xx = pack_feat_linear(feat, false);
						x[node_cnt] = xx;
						node_cnt++;
					}
				}
			}
		}

//		os_feat.close();

	
		linear::parameter _param;
		_param.solver_type = linear::L2R_LR;
		_param.eps = 0.01;
		_param.C = 1;
		_param.nr_weight = 0;
		_param.weight_label = new int(1);
		_param.weight = new double(1.0);
		
		BOOST_FOREACH (unsigned int i, analyze_tags) {
			linear::problem _prob;
			_prob.l = learning_data.size();
			_prob.n = feat2id.size();
			_prob.y = y[i];
			_prob.x = x;
			_prob.bias = -1;
			models[i] = linear::train(&_prob, &_param);
			linear::save_model(model_path[i].string().c_str(), models[i]);
		}
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
									it_tok->mod.tag["source"] = it_eme->second;
								}
								else if (it_eme->first == "time") {
									it_tok->mod.tag["tense"] = it_eme->second;
								}
								else if (it_eme->first == "conditional") {
									it_tok->mod.tag["assumptional"] = it_eme->second;
								}
								else if (it_eme->first == "pmtype") {
									it_tok->mod.tag["type"] = it_eme->second;
								}
								else if (it_eme->first == "actuality") {
									it_tok->mod.tag["authenticity"] = it_eme->second;
								}
								else if (it_eme->first == "evaluation") {
									it_tok->mod.tag["sentiment"] = it_eme->second;
								}
								else if (it_eme->first == "focus") {
									it_tok->mod.tag["focus"] = it_eme->second;
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
		BOOST_FOREACH( t_token tok, *sent ) {
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


	void parser::save_hashDB() {
		f2iDB.close();
		if (!f2iDB.open(f2i_path.string().c_str(), kyotocabinet::HashDB::OCREATE | kyotocabinet::HashDB::OWRITER)) {
			std::cerr << "open error: feat2id: " << f2iDB.error().name() << std::endl;
		}
		f2iDB.clear();

		l2iDB.close();
		if (!l2iDB.open(l2i_path.string().c_str(), kyotocabinet::HashDB::OCREATE | kyotocabinet::HashDB::OWRITER)) {
			std::cerr << "open error: label2id: " << l2iDB.error().name() << std::endl;
		}
		l2iDB.clear();

		boost::unordered_map< std::string, int >::iterator it;
		for (it=feat2id.begin() ; it!=feat2id.end() ; ++it) {
			std::stringstream ss;
			ss << it->second;
			if (!f2iDB.set(it->first, ss.str())) {
				std::cerr << "set error: " << f2iDB.error().name() << std::endl;
			}
		}
		
		for (it=label2id.begin() ; it!=label2id.end() ; ++it) {
			std::stringstream ss;
			ss << it->second;
			if (!l2iDB.set(it->first, ss.str())) {
				std::cerr << "set error: " << l2iDB.error().name() << std::endl;
			}
		}
	}


	void parser::load_hashDB() {
		label2id.clear();
		kyotocabinet::DB::Cursor *cur = l2iDB.cursor();
		cur->jump();
		std::string k, v;
		while (cur->get(&k, &v, true)) {
			label2id[k] = boost::lexical_cast<int>(v);
		}
		
		/*
		feat2id.clear();
		cur = f2iDB.cursor();
		cur->jump();
		while (cur->get(&k, &v, true)) {
			feat2id[k] = boost::lexical_cast<int>(v);
		}
		*/
	}


	void parser::openDB_writable() {
		boost::filesystem::path dir_path[2];
	 	dir_path[0] = l2i_path.parent_path();
		dir_path[1] = f2i_path.parent_path();
		for (unsigned int i=0 ; i<2 ; ++i) {
			boost::system::error_code error;
			const bool result = boost::filesystem::exists(dir_path[i], error);
			if (!result) {
				std::cerr << "mkdir " << dir_path[i].string() << std::endl;
				const bool res = boost::filesystem::create_directories(dir_path[i], error);
				if (!res || error) {
					std::cerr << "ERROR: mkdir " << dir_path[i].string() << " failed" << std::endl;
					exit(-1);
				}
			}
			else if (error) {
				std::cerr << "ERROR" << std::endl;
				exit(-1);
			}
		}

		if (!l2iDB.open(l2i_path.string().c_str(), kyotocabinet::HashDB::OCREATE | kyotocabinet::HashDB::OWRITER)) {
			std::cerr << "open error: label2id: " << l2iDB.error().name() << std::endl;
		}

		if (!f2iDB.open(f2i_path.string().c_str(), kyotocabinet::HashDB::OCREATE | kyotocabinet::HashDB::OWRITER)) {
			std::cerr << "open error: feat2id: " << f2iDB.error().name() << std::endl;
		}
	}


	void parser::openDB() {
		if (!l2iDB.open(l2i_path.string().c_str(), kyotocabinet::HashDB::OREADER)) {
			std::cerr << "open error: label2id: " << l2iDB.error().name() << std::endl;
		}

		if (!f2iDB.open(f2i_path.string().c_str(), kyotocabinet::HashDB::OREADER)) {
			std::cerr << "open error: feat2id: " << f2iDB.error().name() << std::endl;
		}
	}


	void parser::closeDB() {
		if (!l2iDB.close()) {
			std::cerr << "close error: label2id: " << l2iDB.error().name() << std::endl;
		}
		if (!f2iDB.close()) {
			std::cerr << "close error: feat2id: " << f2iDB.error().name() << std::endl;
		}
	}


};



