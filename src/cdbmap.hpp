#ifndef __CDBMAP_HPP__
#define __CDBMAP_HPP__


#include <iostream>
#include <string>
#include <sstream>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include "../cdbpp-1.0/cdbpp.h"

template <typename K, typename V>
class CdbMap {
	private:
		cdbpp::cdbpp dbr;
	public:
		boost::unordered_map<K, V> map;
	public:
		CdbMap(const char *cdb_path) {
			open_cdb(cdb_path);
		}
		CdbMap() {
		}
		~CdbMap() {
		}
		
		void open_cdb(const char *path) {
			if (dbr.is_open()) {
				dbr.close();
			}
			std::ifstream ifs(path);
			dbr.open(ifs);
		}

		bool set(const K key, const V val) {
			map[key] = val;
		}
		
		bool exists_on_map(const K key) {
			if (map.find(key) != map.end()) {
				return true;
			}
			return false;
		}

		bool get(const K key, V *val) {
			if (map.find(key) != map.end()) {
				*val = map[key];
				return true;
			}
			
			if (!dbr.is_open()) {
				return false;
			}

			size_t vsize;
			std::stringstream key_ss;
			key_ss << key;
			const char *value = (const char *)dbr.get(key_ss.str().c_str(), key_ss.str().length(), &vsize);
			if (value == NULL) {
				return false;
			}
			else {
				std::string val_str = std::string(value, vsize);
				*val = boost::lexical_cast<V>(val_str);
				map[key] = *val;
				return true;
			}
		}
		
		size_t size() {
			return map.size();
		}
		
		void dump_map() {
			typename boost::unordered_map<K, V>::iterator it_map;
			for (it_map=map.begin() ; it_map!=map.end() ; ++it_map) {
				std::cout << it_map->first << "\t" << it_map->second << std::endl;
			}
		}
};

#endif

