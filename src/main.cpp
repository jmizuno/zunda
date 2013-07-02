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
		("input,i", boost::program_options::value<int>(), "input layer;\n 0 - raw text [default]\n 1 - cabocha parsed text\n 2 - chapas parsed text")
		("model,m", boost::program_options::value<std::string>(), "model file path (optional): default model.out")
		("pred-rule", "use rule-based event detection")
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

	if (argmap.count("help")) {
		std::cout << opt << std::endl;
		return 1;
	}

	if (argmap.count("version")) {
		std::cout << MODALITY_VERSION << std::endl;
		return 1;
	}

	modality::parser mod_parser;
#if defined (USE_LIBLINEAR)
	mod_parser.load_hashDB();
#endif

	if (argmap.count("pred-rule")) {
		mod_parser.pred_detect_rule = true;
	}

	const boost::filesystem::path path(model_path.c_str());
	boost::system::error_code error;
	const bool result = boost::filesystem::exists(path, error);
	if (!result || error) {
		std::cerr << "ERROR: model file is not found: " << model_path << std::endl;
		exit(-1);
	}

#if defined (USE_LIBLINEAR)
	mod_parser.models[modality::AUTHENTICITY] = linear::load_model(model_path.c_str());
#elif defined (USE_CLASSIAS)
	std::ifstream ifs(model_path.c_str());
	mod_parser.read_model(ifs);
#endif

	std::vector< std::string > sents;
	std::string buf;
	std::string ct;
	while( getline(std::cin, buf) ) {
		switch (input_layer) {
			case modality::raw_text:
				sents.push_back(buf);
				break;
			case modality::cabocha_text:
				ct += buf + "\n";
				if (buf.compare(0, 3, "EOS") == 0) {
					sents.push_back(ct);
					ct.clear();
				}
				break;
			case modality::chapas_text:
				ct += buf + "\n";
				if (buf.compare(0, 3, "EOS") == 0) {
					sents.push_back(ct);
					ct.clear();
				}
				break;
		}
	}

	BOOST_FOREACH ( std::string sent, sents ) {
		nlp::sentence parsed_sent = mod_parser.analyze(sent, input_layer);
		std::cout << parsed_sent.cabocha() << std::endl;
	}

	return 1;
}

