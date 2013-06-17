#ifndef __EVAL_HPP__
#define __EVAL_HPP__

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <iomanip>
#include <boost/format.hpp>
#include "util.hpp"


typedef struct {
	std::string id;
	std::string gold;
	std::string sys;
	double score;
} s_res;


class evaluator {
	public:
		std::vector< s_res > results;
		unsigned int tp;
		unsigned int tn;
		unsigned int fp;
		unsigned int fn;
		unsigned int correct_num;
		unsigned int false_num;
		std::vector< std::string > labels;
		
	public:
		evaluator() {
			correct_num = 0;
			false_num = 0;
		}
		~evaluator() {
		}
		
	private:
		unsigned int count_utf8( const std::string str ) {
			unsigned int size = 0;
			bool first = true;
			std::string tmp;
			for (size_t i=0 ; i<=str.size() ; ++i) {
				if (first || (i != str.size() && (str.at(i) & 0xc0) == 0x80)) {
					tmp += str.at(i);
					first = false;
					continue;
				}
				if (tmp.size() > 1) {
					size += 2;
				}
				else {
					size += 1;
				}
			}

			return size;
		}

	
	
	public:
		void add( std::string id, std::string gold, std::string sys ) {
			s_res res;
			res.id = id;
			res.gold = gold;
			res.sys = sys;
			results.push_back(res);
		}
		
		
		void add( std::string id, std::string gold, std::string sys, double score ) {
			s_res res;
			res.id = id;
			res.gold = gold;
			res.sys = sys;
			res.score = score;
			results.push_back(res);
		}

		
		void eval() {
			correct_num = 0;
			false_num = 0;
			labels.clear();
			BOOST_FOREACH (s_res res, results) {
				if (res.gold == res.sys) {
					correct_num++;
				}
				else {
					false_num++;
				}
				
				if (find(labels.begin(), labels.end(), res.gold) == labels.end()) {
					labels.push_back(res.gold);
				}
				if (find(labels.begin(), labels.end(), res.sys) == labels.end()) {
					labels.push_back(res.sys);
				}
			}
		}

		
		void print_prec_rec() {
			unsigned int width = 0;
			BOOST_FOREACH (std::string label, labels) {
				unsigned int size = count_utf8(label);
				if (size > width) {
					width = size;
				}
			}
			width += 2;

			BOOST_FOREACH (std::string label, labels) {
				for (unsigned int i=0 ; i<(width-count_utf8(label)) ; ++i) {
					std::cout << " ";
				}
				std::cout << label;

				unsigned int tp = 0;
				unsigned int tn = 0;
				unsigned int fp = 0;
				unsigned int fn = 0;

				BOOST_FOREACH (s_res res, results) {
					if (res.sys == label && res.gold == label) {
						tp++;
					}
					else if (res.sys == label && res.gold != label) {
						fp++;
					}
					else if (res.sys != label && res.gold == label) {
						fn++;
					}
					else {
						tn++;
					}
				}
				
				std::cout << " " << boost::format("%4.2lf") % ((double)tp / (double)(tp+fp));
				std::cout << " (" << boost::format("%6d") % tp << "/" << boost::format("%6d") % (tp+fp) << ")";
				std::cout << " " << boost::format("%4.2lf") % ((double)tp / (double)(tp+fn));
				std::cout << " (" << boost::format("%6d") % tp << "/" << boost::format("%6d") % (tp+fn) << ")";
				std::cout << std::endl;
			}
		}

		void print_confusion_matrix() {
			std::cout << "sys\\gold";
			BOOST_FOREACH (std::string label, labels) {
				std::cout << "," << label;
			}
			std::cout << std::endl;

			BOOST_FOREACH (std::string label_sys, labels) {
				std::cout << label_sys;
				unsigned int cnt_sys = 0;
				BOOST_FOREACH (std::string label_gold, labels) {
					unsigned int cnt=0;
					BOOST_FOREACH (s_res res, results) {
						if (res.gold == label_gold && res.sys == label_sys) {
							cnt++;
							cnt_sys++;
						}
					}
					std::cout << "," << cnt;
				}
				std::cout << "," << cnt_sys << std::endl;
			}
		}


		double accuracy() {
			return (double)correct_num / (double)(correct_num + false_num);
		}
};


#endif

