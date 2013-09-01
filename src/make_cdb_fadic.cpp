#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <cabocha.h>

#include "util.hpp"
#include "cdbpp.h"


class Ent {
	public:
		std::string midashi;
		boost::unordered_map< std::string, boost::unordered_map< std::string, std::string > > tag;
		
		Ent() {
		}
		~Ent() {
		}
};


std::string split_chunk(std::string parsed_text) {
	std::vector< std::string > lines;
	boost::algorithm::split( lines, parsed_text, boost::algorithm::is_any_of("\n") );

	std::string chunk = "";
	std::vector< std::string > chunks;
	BOOST_FOREACH(std::string line, lines) {
		if ( line.compare(0, 4, "* 0 ") == 0 ) {
		}
		else if ( line.compare(0, 2, "* ") == 0 ) {
			chunks.push_back(chunk);
			chunk.clear();
		}
		else if ( line.compare(0, 3, "EOS") == 0 ) {
			chunks.push_back(chunk);
		}
		else {
			std::vector< std::string > v;
			boost::algorithm::split( v, line, boost::algorithm::is_any_of("\t") );
			chunk += v[0];
		}
	}

	std::string str;
	join(str, chunks, ".");
	return str;
}

int main( int argc, char *argv[] ) {
	if (argc < 3) {
		std::cerr << "ERROR: use " << argv[0] << " infile outdb" << std::endl;
		return false;
	}

	boost::filesystem::path infile_path(argv[1]);
	boost::filesystem::path outdb_path(argv[2]);
	std::cerr << "infile: " << infile_path.string() << std::endl;
	std::cerr << "outdb:  " << outdb_path.string() << std::endl;

	boost::system::error_code error;
	bool res = boost::filesystem::exists(infile_path, error);
	if (!res || error) {
		std::cerr << "ERROR: no such file \"" << infile_path.string() << "\"" << std::endl;
		return false;
	}

	res = boost::filesystem::exists(outdb_path, error);
	if (res && !error) {
		std::cerr << "ERROR: DB file already exists \"" << outdb_path.string() << "\"" << std::endl;
		return false;
	}

	std::ifstream ifs(infile_path.string().c_str(), std::ios::in);
	CaboCha::Parser *cabocha = CaboCha::createParser("-f1");

	std::vector< Ent > dic;

	std::string line;
	while ( ifs && getline(ifs, line) ) {
		std::vector< std::string > l;
		boost::algorithm::split( l, line, boost::algorithm::is_any_of("\t") );
		if (l.size() != 38) {
			std::cerr << "invalid line > " << line << std::endl;
			continue;
		}
		
		Ent ent;
		ent.midashi = l[4];
		ent.tag["pos_present"]["actuality"] = l[7];
		ent.tag["pos_present"]["worth"] = l[8];
		ent.tag["pos_present"]["sentiment"] = l[9];
		ent.tag["pos_future"]["actuality"] = l[14];
		ent.tag["pos_future"]["worth"] = l[15];
		ent.tag["pos_future"]["sentiment"] = l[16];
		
		dic.push_back(ent);
	}

	std::ofstream ofs(outdb_path.string().c_str(), std::ios_base::binary);
	cdbpp::builder dbw(ofs);

	std::string unk_str = "詳細不明";

	boost::unordered_map< std::string, boost::unordered_map< std::string, std::string > >::iterator it_cond;
	boost::unordered_map< std::string, std::string >::iterator it_tag;
	BOOST_FOREACH(Ent ent, dic) {
		std::string parsed_text = cabocha->parseToString(ent.midashi.c_str());
		std::string chunk = split_chunk(parsed_text);
		for (it_cond=ent.tag.begin() ; it_cond!=ent.tag.end() ; ++it_cond) {
			for (it_tag=it_cond->second.begin() ; it_tag!=it_cond->second.end() ; ++it_tag) {
				std::string key = chunk + ":" + it_cond->first + "_" + it_tag->first;
				std::string val = it_tag->second;
				
				if (val.compare(0, 1, "-") == 0 || val.compare(0, 1, "x") == 0 || val == "" || val.compare(0, unk_str.size(), unk_str) == 0) {
				}
				else {
					std::cerr << key << "\t" << val << std::endl;
					dbw.put(key.c_str(), key.length(), val.c_str(), val.length());
				}
			}
		}
	}
}

