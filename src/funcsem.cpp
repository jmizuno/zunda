#include <iostream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <sstream>

#include "util.hpp"
#ifdef USE_CRFSUITE
#  include <crfsuite.hpp>
#endif
#include "funcsem.hpp"
#include "sentence.hpp"

namespace funcsem {
#ifdef USE_CRFSUITE
	bool tagger::load_model(const std::string &model_dir) {
		boost::filesystem::path model_path(FUNC_MODEL_IPA);
		boost::filesystem::path model_dir_path(model_dir);
		model_path = model_dir_path / model_path;

		if (crf_tagger.open(model_path.string())) {
#ifdef _MODEBUG
			std::cerr << "opened " << model_path.string() << std::endl;
#endif
		}
		else {
			std::cerr << "error: opening " << model_path.string() << std::endl;
			return false;
		}

		return true;
	}


	void tagger::print_labels() {
		boost::unordered_map< std::string, std::vector<std::string> > labels;

		BOOST_FOREACH (std::string label, crf_tagger.labels()) {
			std::vector<std::string> l;
			boost::algorithm::split(l, label, boost::algorithm::is_any_of(":"));
			if (l.size() == 2)
				labels[l[1]].push_back(l[0]);
			else
				labels[l[0]].push_back("");
		}
		boost::unordered_map< std::string, std::vector<std::string> >::iterator it_l, ite_l = labels.end();
		for (it_l=labels.begin() ; it_l!=ite_l ; ++it_l) {
			std::sort(it_l->second.begin(), it_l->second.end());
			std::cerr << it_l->first << "\t" << boost::algorithm::join(it_l->second, ",") << std::endl;
		}
	}


	void tagger::train(const std::string &model_path, const std::vector< nlp::sentence > &tr_data) {
		CRFSuite::Trainer crf_trainer;

		BOOST_FOREACH (nlp::sentence tr_inst, tr_data) {
			std::vector< std::vector< unsigned int > > targets;
			detect_target_gold(tr_inst, targets);

			BOOST_FOREACH (std::vector< unsigned int > tids, targets) {
				unsigned int tid_p = tids[0];
				unsigned int tid_pe = tids[1];
#ifdef _MODEBUG
				std::vector<std::string> surfs;
				for (unsigned int tid=tid_p ; tid<=tid_pe ; ++tid)
					surfs.push_back(tr_inst.get_token(tid)->surf);
				std::string surf = boost::algorithm::join(surfs, ".");
				std::cerr << "range of tokens for " << surf << " " << tid_p << ":" << tid_pe << " / " << tr_inst.tid_min << ":" << tr_inst.tid_max << std::endl;
#endif

				CRFSuite::ItemSequence seq;
				CRFSuite::StringList yseq;
				gen_feat(tr_inst, tid_p, tid_pe, seq);
				for (int tid=tid_p ; tid<=tid_pe ; ++tid)
					yseq.push_back(tr_inst.get_token(tid)->fsem);
				crf_trainer.append(seq, yseq, 0);
			}
		}

		crf_trainer.select("lbfgs", "crf1d");
		BOOST_FOREACH (std::string name, crf_trainer.params())
			std::cout << name << "\t" << crf_trainer.get(name) << "\t" << crf_trainer.help(name) << std::endl;

		crf_trainer.train(model_path, -1);
	}


	/*
	 * * unigram:
	 *   表層、品詞、品詞細分類1、品詞細分類2、品詞細分類3、活用形
	 *   基本形、基本形+品詞、基本形+品詞細分類1、基本形+品詞細分類2
	 * * bigram:
	 *   品詞、表層、基本形、活用形+次の表層
	 */
	void tagger::gen_feat(nlp::sentence &sent, unsigned int tid_p, unsigned int tid_pe, CRFSuite::ItemSequence &seq) {
		const size_t num_uni=10, num_bi=4;

		for (int tid=tid_p ; tid<=tid_pe ; ++tid) {
			std::vector<nlp::token *> toks;
			for (int _tid=tid-2 ; _tid<=tid+2 ; ++_tid) {
				nlp::token *t = sent.get_token(_tid);
				if (t!=NULL) {
					toks.push_back(t);
				}
			}

			std::vector<std::string> feat;

			/* uni-gram feature */
			BOOST_FOREACH (nlp::token *t, toks) {
				int tid_sub = t->id - tid;
				std::stringstream k[num_uni], v[num_uni];
				// surface
				k[0] << "w[" << tid_sub << "]";
				v[0] << t->surf;
				// pos
				k[1] << "p[" << tid_sub << "]";
				v[1] << t->pos;
				// pos1 = pos+pos1
				k[2] << "p[" << tid_sub << "]|p1[" << tid_sub << "]";
				v[2] << t->pos << "|" << t->pos1;
				// pos2 = pos+pos1+pos2
				k[3] << "p[" << tid_sub << "]|p1[" << tid_sub << "]|p2[" << tid_sub << "]";
				v[3] << t->pos << "|" << t->pos1 << "|" << t->pos2;
				// pos3 = pos+pos1+pos2+pos3
				k[4] << "p[" << tid_sub << "]|p1[" << tid_sub << "]|p2[" << tid_sub << "]|p3[" << tid_sub << "]";
				v[4] << t->pos << "|" << t->pos1 << "|" << t->pos2 << "|" << t->pos3;
				// form
				k[5] << "cf[" << tid_sub << "]";
				v[5] << t->form;
				// base form
				k[6] << "bf[" << tid_sub << "]";
				v[6] << t->orig;
				// base form+pos
				k[7] << "bf[" << tid_sub << "]|p[" << tid_sub << "]";
				v[7] << t->orig << "|" << t->pos;
				// base form+pos1
				k[8] << "bf[" << tid_sub << "]|p[" << tid_sub << "]|p1[" << tid_sub << "]";
				v[8] << t->orig << "|" << t->pos << "|" << t->pos1;
				// base form+pos2
				k[9] << "bf[" << tid_sub << "]|p[" << tid_sub << "]|p1[" << tid_sub << "]|p2[" << tid_sub << "]";
				v[9] << t->orig << "|" << t->pos << "|" << t->pos1 << "|" << t->pos2;

				for (size_t i=0 ; i<num_uni ; ++i) {
					if (k[i].str().size() != 0)
						feat.push_back(k[i].str() + "=" + v[i].str());
				}
			}

			/* bi-gram feature */
			std::vector< std::vector<nlp::token *> > ng_toks;
			get_subvec(&ng_toks, toks, 2, 2);
			BOOST_FOREACH (std::vector<nlp::token *> ngt, ng_toks) {
				std::stringstream k[num_bi], v[num_bi];
				BOOST_FOREACH (nlp::token *t, ngt) {
					if (k[0].str().size() != 0) {
						for (size_t i=0 ; i<num_bi ; ++i) {
							k[i] << "|";
							v[i] << "|";
						}
					}
					int tid_sub = t->id - tid;
					// pos
					k[0] << "p[" << tid_sub << "]";
					v[0] << t->pos;
					// surface
					k[1] << "w[" << tid_sub << "]";
					v[1] << t->surf;
					// base form
					k[2] << "bf[" << tid_sub << "]";
					v[2] << t->orig;
					// form+next surface
					if (k[3].str().size() == 0) {
						k[3] << "cf[" << tid_sub << "]";
						v[3] << t->form;
					}
					else {
						k[3] << "w[" << tid_sub << "]";
						v[3] << t->surf;
					}
				}

				for (size_t i=0 ; i<num_bi ; ++i) {
					if (k[i].str().size() != 0)
						feat.push_back(k[i].str() + "=" + v[i].str());
				}
			}

#ifdef _MODEBUG
			std::string feat_str = boost::algorithm::join(feat, " ");
			std::cerr << sent.get_token(tid)->surf << std::endl;
			std::cerr << feat_str << std::endl;
#endif

			CRFSuite::Item item;
			BOOST_FOREACH (std::string _f, feat) {
				CRFSuite::Attribute attr(_f);
				item.push_back(attr);
			}
			seq.push_back(item);
		}
	}


	bool tagger::tag_by_crf(nlp::sentence &sent, unsigned int tid_p, unsigned int tid_pe) {
#ifdef _MODEBUG
		std::vector<std::string> surfs;
		for (unsigned int tid=tid_p ; tid<=tid_pe ; ++tid)
			surfs.push_back(sent.get_token(tid)->surf);
		std::string surf = boost::algorithm::join(surfs, ".");
		std::cerr << "range of tokens for " << surf << " " << tid_p << ":" << tid_pe << " / " << sent.tid_min << ":" << sent.tid_max << std::endl;
#endif

		CRFSuite::ItemSequence seq;
		gen_feat(sent, tid_p, tid_pe, seq);

		CRFSuite::StringList list;
		list = crf_tagger.tag(seq);
		size_t i=0;
		BOOST_FOREACH (std::string r, list) {
#ifdef _MODEBUG
			std::cerr << sent.get_token(i+tid_p)->surf << std::endl;
			std::cerr << r << std::endl;
#endif
			sent.get_token(i+tid_p)->has_fsem = true;
			sent.get_token(i+tid_p)->fsem = r;
			sent.has_fsem = true;
			++i;
		}
	}
#endif


	bool tagger::is_func(const nlp::token &tok) {
		if (tok.pos1 == "非自立" || tok.pos == "助詞" || tok.pos == "助動詞" || std::binary_search(func_terms.begin(), func_terms.end(), tok.orig))
			return true;
		else
			return false;
	}


	bool tagger::is_pred(const nlp::token &tok) {
		if (tok.pos1 != "非自立" &&
			 	((tok.pos == "動詞" && tok.pos1 != "接尾") || tok.pos == "形容詞" || (tok.pos == "名詞" && tok.pos1 == "サ変接続") || (tok.pos=="名詞" && tok.pos1=="形容動詞語幹"))
			 	&& !std::binary_search(func_terms.begin(), func_terms.end(), tok.orig) )
			return true;
		else
			return false;
	}


	void tagger::detect_target_gold(const nlp::sentence &sent, std::vector< std::vector< unsigned int > > &targets) {
		bool find_start = false;
		unsigned int tid_p, tid_pe;
		std::stringstream ss;
		BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
			BOOST_FOREACH (nlp::token tok, chk.tokens) {
				ss << tok.id << ":" << tok.fsem << " ";
				if (find_start == false && tok.has_fsem && tok.fsem.compare(0, 1, "B") == 0) {
					find_start = true;
					tid_p = tok.id;
				}
				if (find_start && (tok.fsem.compare(0, 1, "C") == 0 || tok.fsem.compare(0, 1, "O") == 0)) {
					tid_pe = tok.id;
					std::vector< unsigned int > tids;
					tids.push_back(tid_p);
					tids.push_back(tid_pe-1);
					targets.push_back(tids);
					find_start = false;
				}
			}
		}
		/*
		std::cout << ss.str() << std::endl;
		BOOST_FOREACH (std::vector<unsigned int> tids, targets) {
			std::cout << tids[0] << "\t" << tids[1] << std::endl;
		}
		*/
	}


	void tagger::detect_target(const nlp::sentence &sent, std::vector< std::vector< unsigned int > > &targets) {
		std::vector< unsigned int > tids_pred, tids_normal;
		BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
			BOOST_FOREACH (nlp::token tok, chk.tokens) {
				if (is_pred(tok))
					tids_pred.push_back(tok.id);
				else if (!is_func(tok)) {
					tids_normal.push_back(tok.id);
				}
			}
		}

		/*
		BOOST_FOREACH (unsigned int tid, tids_pred)
			std::cout << "pred " << tid << " " << sent.get_token(tid)->surf << std::endl;
		BOOST_FOREACH (unsigned int tid, tids_normal)
			std::cout << "normal " << tid << " " << sent.get_token(tid)->surf << std::endl;
			*/

		BOOST_FOREACH (unsigned int tid_p, tids_pred) {
			if (tid_p == tids_pred[tids_pred.size()-1] && tid_p != sent.tid_max) {
				std::vector< unsigned int > _tids;
				_tids.push_back(tid_p+1);
				_tids.push_back(sent.tid_max);
				targets.push_back(_tids);
				break;
			}

			unsigned int tid_pe = sent.tid_max;
			BOOST_REVERSE_FOREACH (unsigned int _tid, tids_pred) {
				if (tid_pe >= _tid && _tid > tid_p)
					tid_pe = _tid-1;
			}
			BOOST_REVERSE_FOREACH (unsigned int _tid, tids_normal) {
				if (tid_pe >= _tid && _tid > tid_p)
					tid_pe = _tid-1;
			}

			if (tid_p == tid_pe)
				continue;

			std::vector< unsigned int > tids;
			tids.push_back(tid_p+1);
			tids.push_back(tid_pe);
			targets.push_back(tids);
		}
	}


	void tagger::tag(nlp::sentence &sent) {
		std::vector< std::vector< unsigned int > > targets;
		detect_target(sent, targets);
		sent.has_fsem = true;

		BOOST_FOREACH (std::vector< unsigned int > tids, targets) {
#ifdef USE_CRFSUITE
			tag_by_crf(sent, tids[0], tids[1]);
#endif
		}
	}
};

