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
		("data,d", boost::program_options::value< std::vector<std::string> >()->multitoken(), "directory path containing learning data (required)")
		("input,i", boost::program_options::value<int>(), "format of learning data (optional)\n 0 - cabocha(default)\n 1 - XML")
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

	files.erase(unique(files.begin(), files.end()), files.end());

	if (files.size() == 0) {
		std::cerr << "ERROR: no learn data" << std::endl;
		exit(-1);
	}

	modality::parser mod_parser;
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

	mod_parser.learnOC(model_path, feature_path);

	return 1;
}

