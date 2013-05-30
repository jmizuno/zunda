#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "sentence.hpp"
#include "modality.hpp"

class modxml {
	public:
		std::string out_path;
		boost::filesystem::path path;
};

int main(int argc, char *argv[]) {

	modality::parser mod_parser;

	boost::filesystem::directory_iterator pos("/home/junta-m/work/20110324/TeaM/XML/OC");
	boost::filesystem::directory_iterator last;
	std::vector<boost::filesystem::path> xmls;
	for (; pos!=last ; ++pos) {
		boost::filesystem::path p(*pos);
		xmls.push_back(p);
	}

	boost::filesystem::path out_dir("team");
	std::vector<boost::filesystem::path>::iterator it_xml;
	for (it_xml=xmls.begin() ; it_xml!=xmls.end() ; ++it_xml) {
		std::cerr << it_xml->string()<< std::endl;

		std::vector< std::vector< modality::t_token > > oc_sents = mod_parser.parse_OC(it_xml->string());
		std::vector< nlp::sentence > parsed_sents;
		int cnt = 0;
		BOOST_FOREACH ( std::vector< modality::t_token > oc_sent, oc_sents ) {
			nlp::sentence mod_ipa_sent = mod_parser.make_tagged_ipasents( oc_sent );

			std::stringstream ss;
			ss << (out_dir / it_xml->stem()).string() << "_" << std::setw(3) << std::setfill('0') << cnt << ".depmod";
			std::ofstream os(ss.str().c_str());
			os << mod_ipa_sent.cabocha() << std::endl;
			os.close();
			cnt++;
		}
	}

	return true;
}

