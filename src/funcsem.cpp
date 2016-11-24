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


	void tagger::gen_feat(nlp::sentence &sent, unsigned int tid_p, unsigned int tid_pe, CRFSuite::ItemSequence &seq) {
		const size_t num_uni=10, num_bi=4;

		for (int tid=tid_p+1 ; tid<=tid_pe ; ++tid) {
			std::vector<nlp::token *> toks;
			for (int _tid=tid-2 ; _tid<=tid+2 ; ++_tid) {
				nlp::token *t = sent.get_token(_tid);
				if (t!=NULL) {
					toks.push_back(t);
				}
			}
			/*
			 * [uni-gram]
			 * 0 p,p1: pos+pos1
			 * 1 p,p1,p2: pos+pos1+pos2
			 * 2 p,p1,p2,p3
			 * 3 cf: form
			 * 4 bf,cf: orig+form
			 * [bi-gram]
			 * 0 w: surf
			 * 1 p: pos
			 * 2 bf: orig
			 */
			std::vector<std::string> feat;

			/* uni-gram feature */
			BOOST_FOREACH (nlp::token *t, toks) {
				int tid_sub = t->id - tid;
				std::stringstream k[num_uni], v[num_uni];
				k[0] << "w[" << tid_sub << "]";
				v[0] << t->surf;
				k[1] << "p[" << tid_sub << "]";
				v[1] << t->pos;
				k[2] << "p[" << tid_sub << "]|p1[" << tid_sub << "]";
				v[2] << t->pos << "|" << t->pos1;
				k[3] << "p[" << tid_sub << "]|p1[" << tid_sub << "]|p2[" << tid_sub << "]";
				v[3] << t->pos << "|" << t->pos1 << "|" << t->pos2;
				k[4] << "p[" << tid_sub << "]|p1[" << tid_sub << "]|p2[" << tid_sub << "]|p3[" << tid_sub << "]";
				v[4] << t->pos << "|" << t->pos1 << "|" << t->pos2 << "|" << t->pos3;
				k[5] << "cf[" << tid_sub << "]";
				v[5] << t->form;
				k[6] << "bf[" << tid_sub << "]";
				v[6] << t->orig;
				k[7] << "bf[" << tid_sub << "]|p[" << tid_sub << "]";
				v[7] << t->orig << "|" << t->pos;
				k[8] << "bf[" << tid_sub << "]|p[" << tid_sub << "]|p1[" << tid_sub << "]";
				v[8] << t->orig << "|" << t->pos1;
				k[9] << "bf[" << tid_sub << "]|p[" << tid_sub << "]|p1[" << tid_sub << "]|p2[" << tid_sub << "]";
				v[9] << t->orig << "|" << t->pos1 << "|" << t->pos2;

				for (size_t i=0 ; i<num_uni ; ++i) {
					if (k[i].str().size() != 0)
						feat.push_back(k[i].str() + "=" + v[i].str());
				}
			}

			/* bi-gram feature */
			std::vector< std::vector<nlp::token *> > ng_toks;
			get_subvec(&ng_toks, toks, 1, 2);
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
					k[0] << "p[" << tid_sub << "]";
					v[0] << t->pos;
					k[1] << "w[" << tid_sub << "]";
					v[1] << t->surf;
					k[2] << "bf[" << tid_sub << "]";
					v[2] << t->orig;
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
			std::string feat_str;
			join(feat_str, feat, " ");
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
		std::cerr << "range of tokens for " << sent.get_token(tid_p)->surf << " " << tid_p << ":" << tid_pe << " / " << sent.tid_min << ":" << sent.tid_max << std::endl;
#endif

		CRFSuite::ItemSequence seq;
		gen_feat(sent, tid_p, tid_pe, seq);

		const size_t num_uni=10, num_bi=4;

		CRFSuite::StringList list;
		list = crf_tagger.tag(seq);
		size_t i=0;
		BOOST_FOREACH (std::string r, list) {
#ifdef _MODEBUG
			std::cerr << sent.get_token(i+tid_p+1)->surf << std::endl;
			std::cerr << r << std::endl;
#endif
			sent.get_token(i+tid_p+1)->has_fsem = true;
			sent.get_token(i+tid_p+1)->fsem = r;
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
		if (tok.pos1 != "非自立" && (tok.pos == "動詞" || tok.pos == "形容詞" || (tok.pos == "名詞" && tok.pos1 == "サ変接続") || (tok.pos=="名詞" && tok.pos1=="形容動詞語幹")) && !std::binary_search(func_terms.begin(), func_terms.end(), tok.orig) )
			return true;
		else
			return false;
	}

	void tagger::tag(nlp::sentence &sent, const std::vector<int> &tids) {
		std::vector<unsigned int> tids_pred, tids_normal;
		BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
			BOOST_FOREACH (nlp::token tok, chk.tokens) {
				if (is_pred(tok))
					tids_pred.push_back(tok.id);
				else if (!is_func(tok)) {
					tids_normal.push_back(tok.id);
				}
			}
		}

		BOOST_FOREACH (unsigned int tid_p, tids_pred) {
			if (tid_p == tids_pred[tids_pred.size()-1]) {
				tag_by_crf(sent, tid_p, sent.tid_max);
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

#ifdef USE_CRFSUITE
			tag_by_crf(sent, tid_p, tid_pe);
#endif
		}
	}
};

