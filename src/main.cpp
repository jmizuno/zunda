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
		("model,m", boost::program_options::value<std::string>(), "model file path to store (optional): default [")
		("feature,f", boost::program_options::value<std::string>(), "feature file path to store (optional): default [")
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

	std::string str;
	std::string buf;
	while( getline(std::cin, buf) ) {
		str += buf + "\n";
	}

	modality::parser mod_parser;
//	mod_parser.parse(str);

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

	return 1;
}

