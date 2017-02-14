#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "sentence.hpp"
#include "funcsem.hpp"
#include "util.hpp"


int main(int argc, char *argv[]) {
#if !defined(USE_CRFPP) && !defined(USE_CRFSUITE)
	std::cerr << "ERROR: any crf is not selected in configure" << std::endl;
	exit(-1);
#endif

	boost::program_options::options_description opt("Usage", 200);
	opt.add_options()
		("path,p", boost::program_options::value< std::vector<std::string> >()->multitoken(), "input data to learn (required)")
		("outdir,o", boost::program_options::value<std::string>(), "directory to store output files (optional)\n simple training -  stores model file and feature file to \"model (default)\"\n cross validation - stores model file, feature file and result file to \"output (default)\"")
		("freq", boost::program_options::value<unsigned int>()->default_value(0), "minimum frequency of features (default 0)")
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

#if defined(USE_CRFPP) || defined(USE_CRFSUITE)
	std::string outdir;
	if (argmap.count("outdir")) {
		outdir = argmap["outdir"].as<std::string>();
	}
	else {
		outdir = "model";
	}
	boost::filesystem::path outdir_path(outdir);
	mkdir(outdir_path);

	boost::filesystem::path model_path(FUNC_MODEL_IPA);
	model_path = outdir_path / model_path;
	std::cout << "model file: " << model_path.string() << std::endl;

	std::vector<std::string> files;
	if (argmap.count("path")) {
		BOOST_FOREACH (std::string file, argmap["path"].as< std::vector<std::string> >() ) {
			files.push_back(file);
		}
	}

	sort(files.begin(), files.end());
	files.erase(unique(files.begin(), files.end()), files.end());
	std::cout << "found " << files.size() << " files" << std::endl;

	if (files.size() == 0) {
		std::cerr << "ERROR: no training data" << std::endl;
		exit(-1);
	}

	funcsem::tagger f_tagger;
	std::vector< nlp::sentence > tr_data;

	BOOST_FOREACH (std::string file, files) {
		boost::filesystem::path p(file);
#if BOOST_VERSION >= 104600
		std::string sent_id = p.stem().string();
#elif BOOST_VERSION >= 103600
		std::string sent_id = p.stem();
#endif

		nlp::sentence sent;
		sent.sent_id = sent_id;
		sent.ma_dic = nlp::sentence::IPADic;
		sent.da_tool = nlp::sentence::CaboCha;

		std::ifstream ifs(file.c_str());
		std::string buf, str;
		std::vector< std::string > lines;
		while (getline(ifs,buf)) {
			if (buf != "")
				lines.push_back(buf);
		}
		sent.parse(lines);
		tr_data.push_back(sent);
	}

	std::cout << "load done" << std::endl;
	std::cout << "\t" << tr_data.size() << " sentences" << std::endl;

	unsigned int freq=0;
	if (argmap.count("freq"))
		freq = argmap["freq"].as<unsigned int>();
#if defined(USE_CRFSUITE)
	f_tagger.train_crfsuite(model_path, tr_data, freq);
#elif defined(USE_CRFPP)
	f_tagger.train_crfpp(model_path, tr_data, freq);
#endif

#endif
}

