#ifndef __EVAL_HPP__
#define __EVAL_HPP__

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
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
		

		unsigned int max_width(std::vector<std::string> labels) {
			unsigned int width = 0;
			BOOST_FOREACH (std::string label, labels) {
				unsigned int size = count_utf8(label);
				if (size > width) {
					width = size;
				}
			}
			return width;
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
			unsigned int width = max_width(labels);
			width += 2;

			double prec_sum = 0.0, rec_sum = 0.0;
			BOOST_FOREACH (std::string label, labels) {
				for (unsigned int i=0 ; i<(width-count_utf8(label)) ; ++i) {
					std::cout << " ";
				}
				std::cout << label << ", ";

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
				
				double prec = -1.0, rec = -1.0;
				if (tp+fp == 0) {
					std::cout << std::setw(4) << "-";
				}
				else {
					prec = (double)tp / (double)(tp+fp);
					prec_sum += prec;
					std::cout << boost::format("%4.2lf") % (prec);
				}
				std::cout << ", ";
				std::cout << "(" << boost::format("%6d") % (tp) << "/" << boost::format("%6d") % (tp+fp) << "), ";
					
				if (tp+fn == 0) {
					std::cout << std::setw(4) << "-";
				}
				else {
					rec = (double)tp / (double)(tp+fn);
					rec_sum += rec;
					std::cout << boost::format("%4.2lf") % (rec);
				}
				std::cout << ", ";
				std::cout << "(" << boost::format("%6d") % (tp) << "/" << boost::format("%6d") % (tp+fn) << ")";

				if (prec != -1.0 && rec != -1.0) {
					std::cout << ", " << boost::format("%4.2lf") % (2 * prec * rec / (prec + rec));
				}
				std::cout << std::endl;
			}
			
			double macro_prec = prec_sum / labels.size();
			double macro_rec = rec_sum / labels.size();
			std::cout << "Macro Precision: " << boost::format("%4.2lf") % (macro_prec) << std::endl;
			std::cout << "Macro Recall:    " << boost::format("%4.2lf") % (macro_rec) << std::endl;
			std::cout << "Macro F1:        " << boost::format("%4.2lf") % (2 * macro_prec * macro_rec / (macro_prec + macro_rec)) << std::endl;
		}


		void print_confusion_matrix() {
			unsigned int width = max_width(labels);
			width += 2;
			
			boost::unordered_map<std::string, int> label_w;

			std::string sg = "sys\\gold";
			for (unsigned int i=0 ; i<(width-count_utf8(sg)) ; i++) {
				std::cout << " ";
			}
			std::cout << "sys\\gold";
			BOOST_FOREACH (std::string label, labels) {
				unsigned int w = count_utf8(label);
				if (w<7) {
					label_w[label] = 6;
				}
				else {
					label_w[label] = w;
				}
				
				std::cout << ", ";
				for (unsigned int i=0 ; i<(label_w[label]-w) ; i++) {
					std::cout << " ";
				}
				std::cout << label;
			}
			std::cout << std::endl;

			BOOST_FOREACH (std::string label_sys, labels) {
				for (unsigned int i=0 ; i<(width-count_utf8(label_sys)) ; i++) {
					std::cout << " ";
				}
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
					std::cout << "," << std::setw(label_w[label_gold]+1) << cnt;
				}
				std::cout << ", " << cnt_sys << std::endl;
			}
		}


		double accuracy() {
			return (double)correct_num / (double)(correct_num + false_num);
		}
};


#endif

