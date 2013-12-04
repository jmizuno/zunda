#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "sentence.hpp"
#include "modality.hpp"


int main(int argc, char *argv[]) {
	boost::program_options::options_description opt("Usage", 200);
	opt.add_options()
		("input,i", boost::program_options::value<int>(), "input layer\n 0 - raw text layer [default]\n 1 - dependency parsed layer by CaboCha (IPA POS tag)\n 2 - dependency parsed layer by KNP (Juman POS tag)\n 3 - predicate-argument structure analyzed layer by SynCha (IPA POS tag)\n 4 - predicate-argument structure analyzed layer by KNP (Juman POS tag)")
		("target,t", boost::program_options::value<unsigned int>(), "method of detecting token to be analyzed\n 0 - by part of speech [default]\n 1 - predicate detected by a predicate-argument structure analyzer (only SynCha format is supported)\n 2 - by machine learning (has not been implemented)")
		("model,m", boost::program_options::value<std::string>(), "model directory (optional)")
		("dic,d", boost::program_options::value<std::string>(), "dictionary directory (optional)")
		("help,h", "Show help messages")
		("version,v", "Show version information");

	boost::program_options::variables_map argmap;
	boost::program_options::store(parse_command_line(argc, argv, opt), argmap);
	boost::program_options::notify(argmap);

	int input_layer = 0;
	if (argmap.count("input")) {
		input_layer = argmap["input"].as<int>();
		if (input_layer < modality::RAW || input_layer > modality::KNP_PAS) {
			std::cerr << "invalid input layer" << std::endl;
			exit(-1);
		}
	}

	std::string model_dir;
	switch (input_layer) {
		case modality::RAW:
		case modality::CABOCHA:
		case modality::SYNCHA:
			model_dir = MODELDIR_IPA;
			break;
		case modality::KNP_DEP:
		case modality::KNP_PAS:
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

	if (argmap.count("help")) {
		std::cout << opt << std::endl;
		return 1;
	}

	if (argmap.count("version")) {
		std::cout << MODALITY_VERSION << std::endl;
		return 1;
	}

	modality::parser mod_parser(input_layer, model_dir, dic_dir);

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

	std::string buf;
	std::string sent;
	bool run = false;
	while( getline(std::cin, buf) ) {
		switch (input_layer) {
			case modality::RAW:
				sent = buf;
				run = true;
				break;
			case modality::CABOCHA:
			case modality::SYNCHA:
			case modality::KNP_DEP:
			case modality::KNP_PAS:
				sent += buf +"\n";
				if (buf.compare(0, 3, "EOS") == 0) {
					run = true;
				}
				break;
			default:
				std::cerr << "ERROR: no such input layer" << std::endl;
				return -1;
		}
		
		if (run) {
			std::string parsed_sent = mod_parser.analyzeToString(sent, input_layer);
			std::cout << parsed_sent << std::endl;
			sent.clear();
			run = false;
		}
	}

	return 1;
}

