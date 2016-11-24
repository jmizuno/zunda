#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "sentence.hpp"
#include "modality.hpp"
#include "eval.hpp"
#include "util.hpp"


int main(int argc, char *argv[]) {

	unsigned int split_num = 5;

	boost::program_options::options_description opt("Usage", 200);
	opt.add_options()
		("path,p", boost::program_options::value< std::vector<std::string> >()->multitoken(), "input data to learn (required)")
		("list,l", boost::program_options::value< std::vector<std::string> >()->multitoken(), "list files of input data")
		("input,i", boost::program_options::value<int>(), "input layer (optional)\n 1 - dependency parsed layer by CaboCha/J.DepP\n 2 - dependency parsed layer by KNP\n 3 - predicate-argument structure analyzed layer by SynCha/ChaPAS\n 4 - predicate-argument structure analyzed layer by KNP\n 5 - BCCWJ-style XML format (inputs are analyzed by MeCab and CaboCha with IPA PoS tag)\n 6 - BCCWJ XML format (inputs are analyzed by Juman and KNP)")
		("pos", boost::program_options::value<int>(), "POS tag for CaboCha/J.DepP (optional)\n 0 - IPA/Naist-jdic [default]\n 1 - JumanDic\n 2 - UniDic")
		("posset", boost::program_options::value<std::string>(), "POS set to parse (optional")
		("cross,x", "enable cross validation (optional): default off")
		("split,g", boost::program_options::value<unsigned int>(), "number of groups for cross validation (optional): default 5")
		("outdir,o", boost::program_options::value<std::string>(), "directory to store output files (optional)\n simple training -  stores model file and feature file to \"model (default)\"\n cross validation - stores model file, feature file and result file to \"output (default)\"")
		("dic,d", boost::program_options::value<std::string>(), "dictionary directory (optional)")
		("target,t", boost::program_options::value<unsigned int>(), "method of detecting token to be learned\n 0 - by part of speech [default]\n 1 - predicate detected by a predicate-argument structure analyzer (only SynCha format is supported)\n 2 - by machine learning (has not been implemented)\n 3 - by gold data")
		("only-matrix", "trained and evaluated only matrix clause for each sentence")
		("help,h", "Show help messages")
		("version,v", "Show version informaion");

	boost::program_options::variables_map argmap;
	boost::program_options::store(parse_command_line(argc, argv, opt), argmap);
	boost::program_options::notify(argmap);

	if (argmap.count("help")) {
		std::cout << opt << std::endl;
		return 1;
	}

	if (argmap.count("version")) {
		std::cout << PACKAGE_VERSION << std::endl;
		return 1;
	}

	std::string outdir;
	if (argmap.count("outdir")) {
		outdir = argmap["outdir"].as<std::string>();
	}
	else {
		if (argmap.count("cross")) {
			outdir = "output";
		}
		else {
			outdir = "model";
		}
	}
	boost::filesystem::path outdir_path(outdir);
	mkdir(outdir_path);

	std::string dic_dir = DICDIR;
	if (argmap.count("dic")) {
		dic_dir = argmap["dic"].as<std::string>();
	}
	boost::filesystem::path dic_dir_path(dic_dir);

	int input_layer;
	if (argmap.count("input")) {
		input_layer = argmap["input"].as<int>();
	}

	int pos_tag = modality::POS_IPA;
	if (argmap.count("pos")) {
		pos_tag = argmap["pos"].as<int>();
	}

	std::string pos_set;
	if (argmap.count("posset")) {
		pos_set = argmap["posset"].as<std::string>();
	}

	if (!argmap.count("path") && !argmap.count("list")) {
		std::cerr << "ERROR: no input data" << std::endl;
		return false;
	}
	std::vector<std::string> files;
	if (argmap.count("path")) {
		BOOST_FOREACH (std::string file, argmap["path"].as< std::vector<std::string> >() ) {
			files.push_back(file);
		}
	}
	if (argmap.count("list")) {
		BOOST_FOREACH (std::string list_file, argmap["list"].as< std::vector<std::string> >() ) {
			std::ifstream ifs(list_file.c_str());
			std::string buf;
			while (getline(ifs, buf)) {
				boost::algorithm::trim_right(buf);
				files.push_back(buf);
			}
		}
	}

	sort(files.begin(), files.end());
	files.erase(unique(files.begin(), files.end()), files.end());
	std::cout << "found " << files.size() << " files" << std::endl;

	if (files.size() == 0) {
		std::cerr << "ERROR: no learn data" << std::endl;
		exit(-1);
	}

	modality::parser mod_parser(outdir_path.string(), dic_dir);
	mod_parser.set_pos_tag(pos_tag, pos_set);

	if (argmap.count("target")) {
		mod_parser.target_detection = argmap["target"].as<unsigned int>();
	}

	switch (input_layer) {
		case modality::IN_DEP_CAB:
		case modality::IN_DEP_KNP:
		case modality::IN_PAS_SYN:
		case modality::IN_PAS_KNP:
			mod_parser.load_deppasmods(files, input_layer);
			break;
		case modality::IN_XML_CAB:
		case modality::IN_XML_KNP:
			mod_parser.load_xmls(files, input_layer);
			break;
		default:
			std::cerr << "ERROR: invalid input format" << std::endl;
			exit(-1);
	}

	unsigned int cnt_inst = 0;
	BOOST_FOREACH (nlp::sentence sent, mod_parser.learning_data) {
		BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
			BOOST_FOREACH (nlp::token tok, chk.tokens) {
				if (tok.has_mod) {
					cnt_inst++;
				}
			}
		}
	}

	unsigned int cnt_inst_matrix = 0;
	if (argmap.count("only-matrix")) {
		std::vector<nlp::sentence>::iterator it_sent, ite_sent=mod_parser.learning_data.end();
		for (it_sent=mod_parser.learning_data.begin() ; it_sent!=ite_sent ; ++it_sent) {
			bool use_mod = true;
			std::vector<nlp::chunk>::reverse_iterator it_c, ite_c=it_sent->chunks.rend();
			for (it_c=it_sent->chunks.rbegin() ; it_c!=ite_c ; ++it_c) {
				std::vector<nlp::token>::reverse_iterator it_t, ite_t=it_c->tokens.rend();
				for (it_t=it_c->tokens.rbegin() ; it_t!=ite_t ; ++it_t) {
					if (it_t->has_mod) {
						if (use_mod) {
							cnt_inst_matrix++;
							use_mod = false;
						}
						else {
							it_t->has_mod = false;
							it_t->mod = nlp::modality();
						}
					}
				}
			}
			if (use_mod)
				std::cerr << "not found modality in matrix clause for " << it_sent->sent_id << std::endl;
		}
	}

	std::cout << "load done" << std::endl;
 	std::cout << "   " << mod_parser.learning_data.size() << " sents" << std::endl;
 	std::cout << "   " << cnt_inst << " instances" << std::endl;
	if (argmap.count("only-matrix"))
		std::cout << "   " << cnt_inst_matrix << " instances in matrix clause" << std::endl;
	std::cout << std::endl;


	if (argmap.count("cross")) {
		evaluator evals[LABEL_NUM];
	
		if (argmap.count("split")) {
			split_num = argmap["split"].as<unsigned int>();
		}
		unsigned int grp_size = mod_parser.learning_data.size() / split_num;
		std::vector< std::vector< nlp::sentence > > split_data;

		for (unsigned int i=0 ; i<split_num ; ++i) {
			std::vector< nlp::sentence > grp;
			// last group = rest of learning data
			if (i+1 == split_num) {
				std::copy(mod_parser.learning_data.begin() + i*grp_size, mod_parser.learning_data.end(), std::back_inserter(grp));
			}
			// divided data by split_num
			else {
				std::copy(mod_parser.learning_data.begin() + i*grp_size, mod_parser.learning_data.begin() + (i+1)*grp_size, std::back_inserter(grp));
			}
			split_data.push_back(grp);
		}
		// split_data contains split_num data sets

		unsigned int cnt = 0;
		BOOST_FOREACH (std::vector<nlp::sentence> grp, split_data) {
			std::cout << " group " << cnt << ": " << grp.size() << std::endl;
			cnt++;
		}
		std::cout << std::endl;
		
		for (unsigned int step=0 ; step<split_num ; ++step) {
			std::cout << "* step " << step << std::endl;
			mod_parser.learning_data.clear();

			boost::filesystem::path model_path[LABEL_NUM];
			boost::filesystem::path feat_path[LABEL_NUM];
			boost::filesystem::path result_path[LABEL_NUM];

			BOOST_FOREACH (unsigned int i, mod_parser.analyze_tags) {
				std::stringstream suffix_ss;
				suffix_ss << std::setw(3) << std::setfill('0') << step;
				boost::filesystem::path mp("model_" + mod_parser.id2tag(i) + suffix_ss.str());
				model_path[i] = outdir_path / mp;
				boost::filesystem::path fp("feat_" + mod_parser.id2tag(i) + suffix_ss.str());
				feat_path[i] = outdir_path / fp;
				boost::filesystem::path rp("result_" + mod_parser.id2tag(i) + suffix_ss.str());
				result_path[i] = outdir_path / rp;
			}
			
			for (unsigned int i=0 ; i<split_num ; ++i) {
				if (i != step) {
					// set learning data. used No.step data set as test data
					mod_parser.learning_data.insert(mod_parser.learning_data.end(), split_data[i].begin(), split_data[i].end());
				}
			}
			std::cout << " learning data size: " << mod_parser.learning_data.size() << std::endl;

			mod_parser.learn(model_path, feat_path);

			mod_parser.load_models(model_path);

			std::ofstream *os = new std::ofstream[LABEL_NUM];
			BOOST_FOREACH (unsigned int i, mod_parser.analyze_tags) {
				os[i].open(result_path[i].string().c_str());
			}

			std::vector< nlp::sentence > test_data;
			BOOST_FOREACH (nlp::sentence sent, split_data[step]) {
				test_data.push_back(sent);
			}

			for (unsigned int test_cnt=0 ; test_cnt<test_data.size() ; ++test_cnt) {
#ifdef _MODEBUG
				std::cout << "before " << test_data[test_cnt].sent_id << ": ";
				BOOST_FOREACH (nlp::chunk chk, test_data[test_cnt].chunks) {
					BOOST_FOREACH (nlp::token tok, chk.tokens) {
						if (tok.has_mod) {
							std::string mod_str;
							tok.mod.str(mod_str);
							std::cout << "  " << mod_str << std::endl;
						}
					}
				}
#endif

				// when target detection is DETECT_BY_GOLD, only bool value of having modality is set to test data
				if (mod_parser.target_detection == modality::DETECT_BY_GOLD) {
					BOOST_FOREACH (nlp::chunk chk, test_data[test_cnt].chunks) {
						BOOST_FOREACH (nlp::token tok, chk.tokens) {
							if (tok.has_mod) {
								tok.mod.tag.clear();
							}
						}
					}
				}
				else {
					test_data[test_cnt].clear_mod();
				}

				// tokens to be analyzed are detected by specified method in analyze() and gold data validation
				mod_parser.analyze(test_data[test_cnt]);

#ifdef _MODEBUG
				std::cout << "after  " << test_data[test_cnt].sent_id << ": ";
				BOOST_FOREACH (nlp::chunk chk, test_data[test_cnt].chunks) {
					BOOST_FOREACH (nlp::token tok, chk.tokens) {
						if (tok.has_mod) {
							std::string mod_str;
							tok.mod.str(mod_str);
							std::cout << "  " << mod_str << std::endl;
						}
					}
				}
#endif

				for (unsigned int chk_cnt=0 ; chk_cnt<test_data[test_cnt].chunks.size() ; ++chk_cnt) {
					for (unsigned int tok_cnt=0 ; tok_cnt<test_data[test_cnt].chunks[chk_cnt].tokens.size() ; ++tok_cnt) {
						nlp::token tok_gold = split_data[step][test_cnt].chunks[chk_cnt].tokens[tok_cnt];
						nlp::token tok_sys = test_data[test_cnt].chunks[chk_cnt].tokens[tok_cnt];
						if (tok_gold.has_mod && tok_sys.has_mod) {
							std::stringstream id_ss;
							id_ss << test_data[test_cnt].sent_id << "_" << tok_sys.id;

							BOOST_FOREACH (unsigned int i, mod_parser.analyze_tags) {
								evals[i].add( id_ss.str() , tok_gold.mod.tag[mod_parser.id2tag(i)], tok_sys.mod.tag[mod_parser.id2tag(i)] );
								os[i] << id_ss.str() << "," << tok_gold.mod.tag[mod_parser.id2tag(i)] << "," << tok_sys.mod.tag[mod_parser.id2tag(i)] << std::endl;
							}
						}
						else if (tok_sys.has_mod && !tok_gold.has_mod) {
							std::cerr << "ERROR: modality tag does not exist in token " << tok_sys.id << " in " << test_data[test_cnt].sent_id << std::endl;
						}
					}
				}
			}

			BOOST_FOREACH (unsigned int i, mod_parser.analyze_tags) {
				os[i].close();
			}
		}

		BOOST_FOREACH (unsigned int i, mod_parser.analyze_tags) {
			std::cout << "* " << mod_parser.id2tag(i) << std::endl;
			evals[i].eval();
			double acc = evals[i].accuracy();
			std::cout << acc << std::endl;
			evals[i].print_confusion_matrix();
			evals[i].print_prec_rec();
		}

	}
	else {
		mod_parser.learn();
	}

	mod_parser.save_f2i();
	mod_parser.save_l2i();
	mod_parser.save_i2l();

	return 1;
}

