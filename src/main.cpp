#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "sentence.hpp"
#include "modality.hpp"

int main(int argc, char *argv[]) {

	std::string model_path_def = "model.out";
	std::string feature_path_def = "feature.out";

	boost::program_options::options_description opt("Usage");
	opt.add_options()
		("learn,l", "learn model")
		("input,i", boost::program_options::value<int>(), "input layer;\n -0: raw text [default]\n -1: cabocha parsed text\n -2: chapas parsed text")
		("model,m", boost::program_options::value<std::string>(), "model file path to store (optional): default [")
		("feature,f", boost::program_options::value<std::string>(), "feature file path to store (optional): default [")
		("help,h", "Show help messages")
		("version,v", "Show version informaion");

	boost::program_options::variables_map argmap;
	boost::program_options::store(parse_command_line(argc, argv, opt), argmap);
	boost::program_options::notify(argmap);

	int input_layer = 0;
	if (argmap.count("input")) {
		input_layer = argmap["input"].as<int>();
		if (input_layer < 0 || input_layer > 2) {
			std::cerr << "invalid input layer" << std::endl;
			exit(-1);
		}
	}

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

	bool mode_learn = false;
	if (argmap.count("learn")) {
		mode_learn = true;
	}

//	std::string str;
//	std::string buf;
//	while( getline(std::cin, buf) ) {
//		str += buf + "\n";
//	}

	modality::parser mod_parser;
//	mod_parser.parse(str);


	if (mode_learn) {
		std::cerr << "learn model" << std::endl;
		boost::filesystem::directory_iterator pos("/home/junta-m/work/20110324/TeaM/XML/OC");
		boost::filesystem::directory_iterator last;
		std::vector< std::string > xmls;
		for (; pos!=last ; ++pos) {
			boost::filesystem::path p(*pos);
			xmls.push_back(p.string());
#ifdef DEBUG
			break;
#endif
		}
		mod_parser.learnOC(xmls, model_path, feature_path);
	}
	else {
		if (!argmap.count("model")) {
			std::cerr << "model is needed" << std::endl;
			exit(-1);
		}
		const boost::filesystem::path path(model_path.c_str());
		boost::system::error_code error;
		const bool result = boost::filesystem::exists(path, error);
		if (!result || error) {
			std::cerr << "no such file: " << model_path << std::endl;
			exit(-1);
		}

		std::ifstream ifs(model_path.c_str());

		modality::model_type model;
    classias::quark labels;
    mod_parser.read_model(model, labels, ifs);
		
		std::vector< std::string > sents;
		std::string buf;
		while( getline(std::cin, buf) ) {
			sents.push_back(buf);
		}
		
		BOOST_FOREACH ( std::string sent, sents ) {
			mod_parser.classify(model, labels, sent, input_layer);
		}
	}

	return 1;
}

