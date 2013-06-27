#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include "eval.hpp"


int main(int argc, char *argv[]) {
	boost::program_options::options_description opt("Usage");
	opt.add_options()
		("help,h", "Show help messages")
		("version,v", "Show version informaion");

	boost::program_options::variables_map argmap;
	boost::program_options::store(parse_command_line(argc, argv, opt), argmap);
	boost::program_options::notify(argmap);

	evaluator _evaluator;

	std::string buf;
	while( getline(std::cin, buf) ) {
		std::vector<std::string> l;
		boost::algorithm::split(l, buf, boost::algorithm::is_any_of(","));
		
		if (l.size() == 3) {
			_evaluator.add( l[0], l[1], l[2] );
		}
		else {
			std::cerr << "WARN: invalid format " << buf << std::endl;
		}
	}

	_evaluator.eval();

	std::cout << "* confusion matrix" << std::endl;
	_evaluator.print_confusion_matrix();

	std::cout << "* Precision and Recall" << std::endl;
	_evaluator.print_prec_rec();

	double acc = _evaluator.accuracy();
	std::cout << "* Accuracy" << std::endl;
	std::cout << acc << std::endl;

	return true;
}

