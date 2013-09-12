#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>
//#include <kcpolydb.h>
#include <sstream>

#include "util.hpp"
#include "../cdbpp-1.0/cdbpp.h"


int main( int argc, char *argv[] ) {
	if (argc < 3) {
		std::cerr << "ERROR: use " << argv[0] << " infile outdb" << std::endl;
		return false;
	}

	boost::filesystem::path infile_path(argv[1]);
	boost::filesystem::path outdb_path(argv[2]);
	boost::filesystem::path dump_path(std::string(argv[2]) + ".dump");
	std::cerr << "infile: " << infile_path.string() << std::endl;
	std::cerr << "outdb:  " << outdb_path.string() << std::endl;
	std::cerr << "dumped  " << dump_path.string() << std::endl;

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

	boost::regex surf_sep("\\.");
	boost::regex ignore_class("^[abcdefghijklnstuvwDJNOPQRS]");
	boost::smatch m;

	boost::unordered_map< std::string, std::string > dic;

	std::string buf;
	while ( ifs && getline(ifs, buf) ) {
		std::vector< std::string > v;
		boost::algorithm::split( v, buf, boost::algorithm::is_space() );
		if (v.size() != 2) {
			std::cerr << "invalid line > " << buf << std::endl;
			exit(-1);
		}
		
		std::vector< std::string > funcs;
		
		std::string midashi = v[0];
		std::vector< std::string > ents;
		boost::algorithm::split( ents, v[1], boost::algorithm::is_any_of("/") );
		BOOST_FOREACH( std::string ent, ents ) {
			if ( boost::regex_match(ent, m, ignore_class) ) {
				std::cout << ent << std::endl;
				continue;
			}
			
			std::vector< std::string > attr;
			boost::algorithm::split( attr, ent, boost::algorithm::is_any_of(",") );
			
			std::vector< std::string > surfs;
			for (unsigned int i=2 ; i<attr.size() ; ++i) {
				surfs.push_back(attr[i]);
			}
			std::string surf;
			join(surf, surfs, "_");

			std::vector< std::string > sems;
			boost::algorithm::split( sems, attr[0], boost::algorithm::is_any_of(":") );
			std::string semlabel = sems[0].substr(0,1);
			
			std::vector< std::string > labels;
			boost::algorithm::split( labels, sems[1], boost::algorithm::is_any_of("-") );
			std::string sem = labels[0];
			
			std::stringstream ss;
			ss << surf << "_" << semlabel << "(" << sem << ")";
			
			funcs.push_back(ss.str());
		}
		sort(funcs.begin(), funcs.end());
		funcs.erase(unique(funcs.begin(), funcs.end()), funcs.end());
		std::string func;
		join(func, funcs, "\t");
		dic[midashi] = func;
		//std::cout << midashi << "\t" << funcs.size() << func << std::endl;
	}

	std::ofstream ofs(outdb_path.string().c_str(), std::ios_base::binary);
	cdbpp::builder dbw(ofs);

	std::ofstream ofs_dump(dump_path.string().c_str());
	boost::unordered_map< std::string, std::string >::iterator it_dic;
	for (it_dic=dic.begin() ; it_dic!=dic.end() ; ++it_dic) {
		dbw.put(it_dic->first.c_str(), it_dic->first.length(), it_dic->second.c_str(), it_dic->second.size());
		ofs_dump << it_dic->first << "\t" << it_dic->second << std::endl;
	}
	ofs_dump.close();
}
