#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "sentence.hpp"
#include "modality.hpp"


int main(int argc, char *argv[]) {
#ifndef USE_CRFSUITE
	std::cerr << "WARN: the performance is not good when CRFSuite is disable." << std::endl;
#endif

	std::ios::sync_with_stdio(false);
	std::cin.tie(0);

	boost::program_options::options_description opt("Usage", 200, 100);
	opt.add_options()
		("input,i", boost::program_options::value<int>(), "input layer (optional)\n 0 - raw text layer [default]\n 1 - dependency parsed layer by CaboCha/J.DepP\n 2 - dependency parsed layer by KNP\n 3 - predicate-argument structure analyzed layer by SynCha/ChaPAS\n 4 - predicate-argument structure analyzed layer by KNP")
		("pos", boost::program_options::value<int>(), "POS tag for CaboCha/J.DepP (optional)\n 0 - IPA/Naist-jdic [default]\n 1 - JumanDic")
		("posset", boost::program_options::value<std::string>(), "POS set to parse (optional)")
		("target,t", boost::program_options::value<unsigned int>(), "method of detecting token to be analyzed\n 0 - by part of speech [default]\n 1 - predicate detected by a predicate-argument structure analyzer (only SynCha format is supported)\n 2 - by machine learning (has not been implemented)\n 3 - detected by #EVENT line")
		("model,m", boost::program_options::value<std::string>(), "model directory (optional)")
		("dic,d", boost::program_options::value<std::string>(), "dictionary directory (optional)")
		("info", "print information of model files")
		("help,h", "Show help messages")
		("version,v", "Show version information");

	/*
	boost::program_options::options_description opt("Usage", 200);
	opt.add_options()
		("input,i", boost::program_options::value<int>(), "input layer\n 0 - raw text layer [default]\n 1 - dependency parsed layer by CaboCha (IPA POS tag)\n 2 - dependency parsed layer by KNP (Juman POS tag)\n 3 - predicate-argument structure analyzed layer by SynCha (IPA POS tag)\n 4 - predicate-argument structure analyzed layer by KNP (Juman POS tag)")
		("target,t", boost::program_options::value<unsigned int>(), "method of detecting token to be analyzed\n 0 - by part of speech [default]\n 1 - predicate detected by a predicate-argument structure analyzer (only SynCha format is supported)\n 2 - by machine learning (has not been implemented)")
		("model,m", boost::program_options::value<std::string>(), "model directory (optional)")
		("dic,d", boost::program_options::value<std::string>(), "dictionary directory (optional)")
		("help,h", "Show help messages")
		("version,v", "Show version information");
		*/

	boost::program_options::variables_map argmap;
	boost::program_options::store(parse_command_line(argc, argv, opt), argmap);
	boost::program_options::notify(argmap);

	int input_layer = 0;
	if (argmap.count("input")) {
		input_layer = argmap["input"].as<int>();
		if (input_layer < modality::IN_RAW || input_layer > modality::IN_PAS_KNP) {
			std::cerr << "invalid input layer" << std::endl;
			exit(-1);
		}
	}

	int pos_tag = modality::POS_IPA;
	if (argmap.count("pos")) {
		pos_tag = argmap["pos"].as<int>();
	}

	std::string pos_set;
	if (argmap.count("posset")) {
		pos_set = argmap["posset"].as<std::string>();
	}

	std::string model_dir;
	switch (pos_tag) {
		case modality::POS_IPA:
			model_dir = MODELDIR_IPA;
			break;
		case modality::POS_JUMAN:
			model_dir = MODELDIR_JUMAN;
			break;
		default:
			std::cerr << "ERROR: no such input layer" << std::endl;
			return -1;
	}

	if (argmap.count("model")) {
		model_dir = argmap["model"].as<std::string>();
	}

	std::string dic_dir = DICDIR;
	if (argmap.count("dic")) {
		dic_dir = argmap["dic"].as<std::string>();
	}
	boost::filesystem::path dic_dir_path(dic_dir);

	if (argmap.count("help")) {
		std::cout << opt << std::endl;
		return 1;
	}

	if (argmap.count("version")) {
		std::cout << PACKAGE_VERSION << std::endl;
		return 1;
	}

	modality::parser mod_parser(model_dir, dic_dir);
	mod_parser.set_pos_tag(pos_tag, pos_set);

	if (argmap.count("info")) {
		std::cout << "* model files" << std::endl;
		for (unsigned int i=1 ; i<LABEL_NUM ; ++i) {
			std::time_t lu = boost::filesystem::last_write_time(mod_parser.model_path[i]);
			boost::posix_time::ptime lu_t = boost::posix_time::from_time_t(lu);
			std::cout << "  " << lu_t << "\t" << mod_parser.model_path[i].string() << std::endl;
		}
		return true;
	}

	if (argmap.count("target")) {
		mod_parser.target_detection = argmap["target"].as<unsigned int>();
	}

#ifdef _MODEBUG
	clock_t st;
	clock_t et;
	st = std::clock();
#endif
	if (!mod_parser.load_models()) {
		std::cerr << "ERROR: load models failed" << std::endl;
		return false;
	}
#ifdef _MODEBUG
	et = std::clock();
	std::cerr << "* load model done: " << (et-st) / (double)CLOCKS_PER_SEC << " sec" << std::endl;
#endif

#ifdef USE_CRFSUITE
#ifdef _MODEBUG
	st = std::clock();
#endif
	if (!mod_parser.f_tagger->load_model(model_dir)) {
		std::cerr << "ERROR: load model failed" << std::endl;
		return false;
	}
#ifdef _MODEBUG
	//mod_parser.f_tagger->print_labels();

	et = std::clock();
	std::cerr << "* load model done: " << (et-st) / (double)CLOCKS_PER_SEC << " sec" << std::endl;
#endif
#endif

	std::string buf;
	std::vector<std::string> sent;
	std::string in_sent, parsed_sent;
	bool run = false;
	while( getline(std::cin, buf) ) {
		if (buf == "") {
			continue;
		}
		boost::algorithm::trim_right(buf);
		switch (input_layer) {
			case modality::IN_RAW:
				in_sent = buf;
				run = true;
				break;
			case modality::IN_DEP_CAB:
			case modality::IN_DEP_KNP:
			case modality::IN_PAS_SYN:
			case modality::IN_PAS_KNP:
				if (buf.compare(0, 3, "EOS") == 0) {
					sent.push_back(buf);
					in_sent = boost::algorithm::join(sent, "\n");
					run = true;
				}
				else {
					sent.push_back(buf);
				}
				break;
			default:
				std::cerr << "ERROR: no such input layer" << std::endl;
				return -1;
		}
		
		if (run) {
		 	mod_parser.analyzeToString(in_sent, input_layer, parsed_sent);
			boost::algorithm::trim_right(parsed_sent);
			std::cout << parsed_sent << std::endl;
			sent.clear();
			in_sent.clear();
			run = false;
		}
	}

	return 1;
}

