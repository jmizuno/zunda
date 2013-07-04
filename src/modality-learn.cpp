#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "sentence.hpp"
#include "modality.hpp"
#include "eval.hpp"


int main(int argc, char *argv[]) {

	std::string model_path_def = "model.out";
	std::string feature_path_def = "feature.out";
	unsigned int split_num = 5;

	boost::program_options::options_description opt("Usage");
	opt.add_options()
		("data,d", boost::program_options::value< std::vector<std::string> >()->multitoken(), "directory path containing learning data (required)")
		("input,i", boost::program_options::value<int>(), "format of learning data (optional)\n 0 - cabocha(default)\n 1 - XML")
		("ext,e", boost::program_options::value<std::string>(), "extension of learning files (optional): default: 0 - .deppasmod, 1 - .xml")
		("cross,x", "enable cross validation (optional): default off")
		("split,g", boost::program_options::value<unsigned int>(), "number of groups for cross-validation (optional): default 5")
		("outdir,o", boost::program_options::value<std::string>(), "directory path to output results of cross validation including feature, model and classified resutls (optional): default: output")
		("model,m", boost::program_options::value<std::string>(), "model file path (optional): default model.out")
		("feature,f", boost::program_options::value<std::string>(), "feature file path (optional): default feature.out")
		("help,h", "Show help messages")
		("version,v", "Show version informaion");

	boost::program_options::variables_map argmap;
	boost::program_options::store(parse_command_line(argc, argv, opt), argmap);
	boost::program_options::notify(argmap);

	std::string model_path = model_path_def;
	if (argmap.count("model")) {
		model_path = argmap["model"].as<std::string>();
	}

	std::string feature_path = feature_path_def;
	if (argmap.count("feature")) {
		feature_path = argmap["feature"].as<std::string>();
	}

	if (argmap.count("help")) {
		std::cout << opt << std::endl;
		return 1;
	}

	if (argmap.count("version")) {
		std::cout << MODALITY_VERSION << std::endl;
		return 1;
	}

	int input_layer = 0;
	if (argmap.count("input")) {
		input_layer = argmap["input"].as<int>();
	}
	std::string ext;
	switch (input_layer) {
		case 0:
			std::cout << "data format: cabocha" << std::endl;
			ext = ".deppasmod";
			break;
		case 1:
			std::cout << "data format: XML" << std::endl;
			ext = ".xml";
			break;
		default:
			std::cerr << "ERROR: invalid data format type" << std::endl;
			exit(-1);
	}
	if (argmap.count("ext")) {
		ext = argmap["ext"].as<std::string>();
		if (ext.compare(0, 1, ".") != 0) {
			ext = "." + ext;
		}
	}

	if (!argmap.count("data")) {
		std::cerr << "ERROR: no data directory" << std::endl;
		exit(-1);
	}

	std::vector<std::string> files;
	BOOST_FOREACH (std::string learn_data_dir, argmap["data"].as< std::vector<std::string> >() ) {
		const boost::filesystem::path learn_data_path(learn_data_dir);
		if (boost::filesystem::exists(learn_data_path)) {
			BOOST_FOREACH ( const boost::filesystem::path& p, std::make_pair(boost::filesystem::recursive_directory_iterator(learn_data_dir), boost::filesystem::recursive_directory_iterator()) ) {
				if (p.extension() == ext) {
					files.push_back(p.string());
				}
			}
		}
		else {
			std::cerr << "ERROR: no such directory: " << learn_data_dir << std::endl;
			exit(-1);
		}
	}

	sort(files.begin(), files.end());
	files.erase(unique(files.begin(), files.end()), files.end());

	if (files.size() == 0) {
		std::cerr << "ERROR: no learn data" << std::endl;
		exit(-1);
	}

	modality::parser mod_parser;
	mod_parser.load_hashDB();

	switch (input_layer) {
		case 0:
			mod_parser.load_deppasmods(files);
			break;
		case 1:
			mod_parser.load_xmls(files);
			break;
		default:
			std::cerr << "ERROR: invalid data format type" << std::endl;
			exit(-1);
	}
	std::cout << "load done: " << mod_parser.learning_data.size() << " sents" << std::endl;

	if (argmap.count("cross")) {
		evaluator evals[LABEL_NUM];
		
		std::string outdir = "output";
		if (argmap.count("outdir")) {
			outdir = argmap["outdir"].as<std::string>();
		}
		boost::filesystem::path outdir_path(outdir);
		
		boost::system::error_code error;
		const bool result = boost::filesystem::exists(outdir_path, error);
		if (!result) {
			std::cerr << "mkdir " << outdir_path.string() << std::endl;
			const bool res = boost::filesystem::create_directories(outdir_path, error);
			if (!res || error) {
				std::cerr << "ERROR: mkdir " << outdir_path.string() << " failed" << std::endl;
				exit(-1);
			}
		}
		else if (error) {
			std::cerr << "ERROR" << std::endl;
			exit(-1);
		}

		if (argmap.count("x")) {
			split_num = argmap["x"].as<unsigned int>();
		}
		unsigned int grp_size = mod_parser.learning_data.size() / split_num;
		std::vector< std::vector< nlp::sentence > > split_data;

		for (unsigned int i=0 ; i<split_num ; ++i) {
			std::vector< nlp::sentence > grp;
			if (i+1 == split_num) {
				std::copy(mod_parser.learning_data.begin() + i*grp_size, mod_parser.learning_data.end(), std::back_inserter(grp));
			}
			else {
				std::copy(mod_parser.learning_data.begin() + i*grp_size, mod_parser.learning_data.begin() + (i+1)*grp_size, std::back_inserter(grp));
			}
			split_data.push_back(grp);
		}

		unsigned int cnt = 0;
		BOOST_FOREACH (std::vector<nlp::sentence> grp, split_data) {
			std::cout << " group " << cnt << ": " << grp.size() << std::endl;
			cnt++;
		}
		std::cout << std::endl;
		

		
		for (unsigned int step=0 ; step<split_num ; ++step) {
			std::cout << "* step " << step << std::endl;
			mod_parser.learning_data.clear();
			
			std::string model_path[LABEL_NUM];
			std::string feature_path[LABEL_NUM];
			std::string result_path[LABEL_NUM];
			
			for (unsigned int i=0 ; i<LABEL_NUM ; ++i) {
				std::string label = mod_parser.id2tag(i);
			
				std::stringstream suffix_ss;
				suffix_ss << std::setw(3) << std::setfill('0') << step;
				boost::filesystem::path mp("model_" + label + suffix_ss.str());
				model_path[i] = (outdir_path / mp).string();
				boost::filesystem::path fp("feature_" + label + suffix_ss.str());
				feature_path[i] = (outdir_path / fp).string();
				boost::filesystem::path rp("result_" + label + suffix_ss.str());
				result_path[i] = (outdir_path / rp).string();
			}
			
			for (unsigned int i=0 ; i<split_num ; ++i) {
				if (i != step) {
					mod_parser.learning_data.insert(mod_parser.learning_data.end(), split_data[i].begin(), split_data[i].end());
				}
			}
			std::cout << " learning data size: " << mod_parser.learning_data.size() << std::endl;
			
			mod_parser.learn(model_path, feature_path);

			mod_parser.load_models(model_path);

			std::ofstream *os = new std::ofstream[LABEL_NUM];
			for (unsigned int i=0 ; i<LABEL_NUM ; ++i) {
				os[i].open(result_path[i].c_str());
			}

			std::vector< nlp::sentence > test_data = split_data[step];
			for (unsigned int set=0 ; set<test_data.size() ; ++set) {
				test_data[set].clear_mod();
				nlp::sentence tagged_sent = mod_parser.analyze(test_data[set]);
				for (unsigned int chk_cnt=0 ; chk_cnt<tagged_sent.chunks.size() ; ++chk_cnt) {
					for (unsigned int tok_cnt=0 ; tok_cnt<tagged_sent.chunks[chk_cnt].tokens.size() ; ++tok_cnt) {
						nlp::token tok_gold = split_data[step][set].chunks[chk_cnt].tokens[tok_cnt];
						nlp::token tok_sys = tagged_sent.chunks[chk_cnt].tokens[tok_cnt];
						if (tok_gold.has_mod && tok_sys.has_mod) {
							std::stringstream id_ss;
							id_ss << tagged_sent.sent_id << "_" << tok_sys.id;

							for (unsigned int i=0 ; i<LABEL_NUM ; ++i) {
								evals[i].add( id_ss.str() , tok_gold.mod.tag[mod_parser.id2tag(i)], tok_sys.mod.tag[mod_parser.id2tag(i)] );
								os[i] << id_ss.str() << "," << tok_gold.mod.tag[mod_parser.id2tag(i)] << "," << tok_sys.mod.tag[mod_parser.id2tag(i)] << std::endl;
							}
						}
						else if (tok_sys.has_mod && !tok_gold.has_mod) {
							std::cerr << "ERROR: modality tag does not exist in token " << tok_sys.id << " in " << test_data[set].sent_id << std::endl;
						}
					}
				}
			}

			for (unsigned int i=0 ; i<LABEL_NUM ; ++i) {
				os[i].close();
			}

		}

		for (unsigned int i=0 ; i<LABEL_NUM ; ++i) {
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

	mod_parser.save_hashDB();

	return 1;
}

