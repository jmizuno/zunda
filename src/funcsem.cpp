#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>

#include "util.hpp"
#ifdef USE_CRFSUITE
#  include <crfsuite.hpp>
#endif
#include <crfpp.h>
#include "funcsem.hpp"
#include "sentence.hpp"

namespace funcsem {
	void tagger::set_model_dir(const std::string &dir) {
		boost::filesystem::path dir_path(dir);
		set_model_dir(dir_path);
	}

	void tagger::set_model_dir(const boost::filesystem::path &dir_path) {
		boost::filesystem::path mp(FUNC_MODEL_IPA);
		model_path = dir_path / mp;
	}

	bool tagger::load_model(const std::string &model_dir) {
		boost::filesystem::path mp(FUNC_MODEL_IPA);
		boost::filesystem::path md(model_dir);
		model_path = mp / md;

		return load_model();
	}

	bool tagger::load_model(const boost::filesystem::path &mp) {
		model_path = mp;
		return load_model();
	}

	bool tagger::load_model() {
#if defined(USE_CRFSUITE)
		return load_model_crfsuite();
#elif defined(USE_CRFPP)
		return load_model_crfpp();
#endif
	}


#if defined(USE_CRFPP)
	bool tagger::load_model_crfpp() {
		std::vector<const char*> argv;
		argv.push_back("crf_test");
		argv.push_back("-m");
		argv.push_back(model_path.string().c_str());
		fsem_tagger_crfpp_model = crfpp_model_new(argv.size(), const_cast<char **>(&argv[0]));
		fsem_tagger_crfpp = crfpp_model_new_tagger(fsem_tagger_crfpp_model);
		crfpp_clear(fsem_tagger_crfpp);
#ifdef _MODEBUG
		crfpp_set_vlevel(fsem_tagger_crfpp, 1);
#endif

#ifdef _MODEBUG
		std::cerr << "loaded " << model_path.string() << ": " << boost::filesystem::file_size(model_path) << " byte, " << boost::posix_time::from_time_t(boost::filesystem::last_write_time(model_path)) << std::endl;
#endif

		return true;
	}
#endif


#if defined(USE_CRFPP)
	void tagger::tag_by_crfpp(nlp::sentence &sent, unsigned int tid_p, unsigned int tid_pe) {
#ifdef _MODEBUG
		std::vector<std::string> surfs;
		for (unsigned int tid=tid_p ; tid<=tid_pe ; ++tid)
			surfs.push_back(sent.get_token(tid)->surf);
		std::string surf = boost::algorithm::join(surfs, ".");
		std::cerr << "range of tokens for " << surf << " " << tid_p << ":" << tid_pe << " / " << sent.tid_min << ":" << sent.tid_max << std::endl;
#endif

		unsigned int tid_p2=tid_p, tid_pe2=tid_pe;
		if (tid_p2 < 2)
			tid_p2 = 0;
		else
			tid_p2 -= 2;
		if ( (sent.tid_max - tid_pe2) <= 2)
			tid_pe2 = sent.tid_max;
		else
			tid_pe2 += 2;
		//std::cout << tid_p2 << " " << tid_p << " " << tid_pe << " " << tid_pe2 << " " << sent.tid_max << std::endl;
		for (unsigned int tid=tid_p2 ; tid<=tid_pe2 ; ++tid) {
			std::vector<const char *> feature;
			nlp::token *t = sent.get_token(tid);

			feature.push_back(t->surf.c_str());
			feature.push_back(t->pos.c_str());
			feature.push_back(t->pos1.c_str());
			feature.push_back(t->pos2.c_str());
			feature.push_back(t->pos3.c_str());
			feature.push_back(t->type.c_str());
			feature.push_back(t->form.c_str());
			feature.push_back(t->orig.c_str());
			feature.push_back(t->read.c_str());
			feature.push_back(t->pron.c_str());

			crfpp_add2(fsem_tagger_crfpp, feature.size(), (const char **)&(feature[0]));
		}

		crfpp_parse(fsem_tagger_crfpp);
#ifdef _MODEBUG
		std::string verbose(crfpp_tostr(fsem_tagger_crfpp));
		std::cerr << verbose << std::endl;
#endif

		unsigned int len = tid_pe2 - tid_p2;
		for (unsigned int i=0 ; i<=len ; ++i) {
			unsigned int tid = tid_p2 + i;
			if (tid_p <= tid && tid <= tid_pe) {
				nlp::token *t = sent.get_token(tid);
				std::string fsem( crfpp_y2(fsem_tagger_crfpp, i) );
				t->has_fsem = true;
				t->fsem = fsem;
			}
		}

		crfpp_clear(fsem_tagger_crfpp);
	}
#endif


#if defined(USE_CRFPP)
	void tagger::train_crfpp(const std::string &model_path, const std::vector< nlp::sentence > &tr_data, unsigned int freq) {
		boost::filesystem::path mp(model_path);
		train_crfpp(mp, tr_data, freq);
	}


	void tagger::train_crfpp(const boost::filesystem::path &mp, const std::vector< nlp::sentence > &tr_data, unsigned int freq) {

		std::string tmpl_crfpp = \
"U00:%x[-2,0]\n\
U01:%x[-2,1]\n\
U02:%x[-2,1]/%x[-2,2]\n\
U03:%x[-2,1]/%x[-2,2]/%x[-2,3]\n\
U04:%x[-2,1]/%x[-2,2]/%x[-2,3]/%x[-2,4]\n\
U05:%x[-2,7]\n\
U06:%x[-2,8]\n\
U07:%x[-2,8]/%x[-2,1]\n\
U08:%x[-2,8]/%x[-2,2]\n\
U09:%x[-2,8]/%x[-2,3]\n\
\n\
U10:%x[-1,0]\n\
U11:%x[-1,1]\n\
U12:%x[-1,1]/%x[-1,2]\n\
U13:%x[-1,1]/%x[-1,2]/%x[-1,3]\n\
U14:%x[-1,1]/%x[-1,2]/%x[-1,3]/%x[-1,4]\n\
U15:%x[-1,7]\n\
U16:%x[-1,8]\n\
U17:%x[-1,8]/%x[-1,1]\n\
U18:%x[-1,8]/%x[-1,2]\n\
U19:%x[-1,8]/%x[-1,3]\n\
\n\
U20:%x[0,0]\n\
U21:%x[0,1]\n\
U22:%x[0,1]/%x[0,2]\n\
U23:%x[0,1]/%x[0,2]/%x[0,3]\n\
U24:%x[0,1]/%x[0,2]/%x[0,3]/%x[0,4]\n\
U25:%x[0,7]\n\
U26:%x[0,8]\n\
U27:%x[0,8]/%x[0,1]\n\
U28:%x[0,8]/%x[0,2]\n\
U29:%x[0,8]/%x[0,3]\n\
\n\
U30:%x[1,0]\n\
U31:%x[1,1]\n\
U32:%x[1,1]/%x[1,2]\n\
U33:%x[1,1]/%x[1,2]/%x[1,3]\n\
U34:%x[1,1]/%x[1,2]/%x[1,3]/%x[1,4]\n\
U35:%x[1,7]\n\
U36:%x[1,8]\n\
U37:%x[1,8]/%x[1,1]\n\
U38:%x[1,8]/%x[1,2]\n\
U39:%x[1,8]/%x[1,3]\n\
\n\
U40:%x[2,0]\n\
U41:%x[2,1]\n\
U42:%x[2,1]/%x[2,2]\n\
U43:%x[2,1]/%x[2,2]/%x[2,3]\n\
U44:%x[2,1]/%x[2,2]/%x[2,3]/%x[2,4]\n\
U45:%x[2,7]\n\
U46:%x[2,8]\n\
U47:%x[2,8]/%x[2,1]\n\
U48:%x[2,8]/%x[2,2]\n\
U49:%x[2,8]/%x[2,3]\n\
\n\
\n\
B00:%x[-2,1]/%x[-1,1]\n\
B01:%x[-1,1]/%x[0,1]\n\
B02:%x[0,1]/%x[1,1]\n\
B03:%x[1,1]/%x[2,1]\n\
\n\
B10:%x[-2,0]/%x[-1,0]\n\
B11:%x[-1,0]/%x[0,0]\n\
B12:%x[0,0]/%x[1,0]\n\
B13:%x[1,0]/%x[2,0]\n\
\n\
B20:%x[-2,8]/%x[-1,8]\n\
B21:%x[-1,8]/%x[0,8]\n\
B22:%x[0,8]/%x[1,8]\n\
B23:%x[1,8]/%x[2,8]\n\
\n\
B30:%x[-2,7]/%x[-1,0]\n\
B31:%x[-1,8]/%x[0,0]\n\
B32:%x[0,8]/%x[1,0]\n\
B33:%x[1,8]/%x[2,0]\n";

		std::vector<std::string> out;

		boost::filesystem::path train_path = boost::filesystem::temp_directory_path() / boost::filesystem::path("zunda-funcsem-train.crfpp");
		boost::filesystem::path tmpl_path = boost::filesystem::temp_directory_path() / boost::filesystem::path("zunda-funcsem-train.crfpp.tmpl");

		std::cout << "training file: " << train_path << std::endl;
		std::cout << "template file: " << tmpl_path << std::endl;
		std::cout << "model file:    " << mp << std::endl;

		BOOST_FOREACH (nlp::sentence sent, tr_data) {
			unsigned int tid_s, tid_e;
			bool flag_b = false;
			BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
				BOOST_FOREACH (nlp::token tok, chk.tokens) {
					if (tok.fsem.compare(0, 1, "B") == 0) {
						if (flag_b) {
							continue;
						}
						else {
							tid_s = tok.id;
							flag_b = true;
						}
					}
					else if ( flag_b && (tok.fsem.compare(0,1,"C") == 0 || tok.fsem.compare(0,1,"O") == 0) ) {
						unsigned int tid_s_ext, tid_e_ext;
						tid_e = tok.id-1;
						if (tid_s < 2)
							tid_s_ext = 0;
						else
							tid_s_ext = tid_s - 2;
						if (tid_e + 2 >= sent.tid_max)
							tid_e_ext = sent.tid_max;
						else
							tid_e_ext = tid_e + 2;
						//std::cout << tid_s_ext << " " << tid_s << " " << tid_e << " " << tid_e_ext << " " << sent.tid_max << std::endl;

						for (unsigned int i=tid_s_ext ; i<tid_e_ext+1 ; ++i) {
							nlp::token *t_i = sent.get_token(i);
							std::string feat(t_i->surf + " " + t_i->pos + " " + t_i->pos1 + " " + t_i->pos2 + " " + t_i->pos3 + " " + t_i->type + " " + t_i->form + " " + t_i->orig + " " + t_i->read + " " + t_i->pron);
							if (t_i->fsem.compare(0,1,"B") == 0 || t_i->fsem.compare(0,1,"I") == 0) {
								if (tid_s <= i && i <= tid_e)
									out.push_back(feat + " " + t_i->fsem);
							}
							else
								out.push_back(feat + " O");
						}
						out.push_back("");
						flag_b = false;
					}
				}
			}
		}

		std::ofstream ofs;
		ofs.open(train_path.c_str());
		BOOST_FOREACH (std::string line, out)
			ofs << line << std::endl;
		ofs.close();

		ofs.open(tmpl_path.c_str());
		ofs << tmpl_crfpp << std::endl;
		ofs.close();

		std::vector<const char *> argv;
		argv.push_back("crf_learn");
		std::stringstream ss;
		ss << "--freq=" << freq+1 << std::endl;
		argv.push_back(ss.str().c_str());
		argv.push_back("-t");
		argv.push_back(tmpl_path.c_str());
		argv.push_back(train_path.c_str());
		argv.push_back(mp.c_str());
		crfpp_learn(static_cast<int>(argv.size()), const_cast<char **>(&argv[0]));

#ifndef _MODEBUG
		boost::filesystem::remove(tmpl_path);
		boost::filesystem::remove(train_path);
#endif
	}
#endif


#if defined(USE_CRFSUITE)
	bool tagger::load_model_crfsuite() {
		if (fsem_tagger_crfs.open(model_path.string())) {
#ifdef _MODEBUG
			std::cerr << "loaded " << model_path.string() << ": " << boost::filesystem::file_size(model_path) << " byte, " << boost::posix_time::from_time_t(boost::filesystem::last_write_time(model_path)) << std::endl;
#endif
		}
		else {
			std::cerr << "error: opening " << model_path.string() << std::endl;
			return false;
		}

		return true;
	}
#endif


#if defined(USE_CRFSUITE)
	void tagger::print_labels_crfsuite() {
		boost::unordered_map< std::string, std::vector<std::string> > labels;

		BOOST_FOREACH (std::string label, fsem_tagger_crfs.labels()) {
			std::vector<std::string> l;
			boost::algorithm::split(l, label, boost::algorithm::is_any_of(":"));
			if (l.size() == 2)
				labels[l[1]].push_back(l[0]);
			else
				labels[l[0]].push_back("");
		}
		boost::unordered_map< std::string, std::vector<std::string> >::iterator it_l, ite_l = labels.end();
		std::vector<std::string> lbs;
		for (it_l=labels.begin() ; it_l!=ite_l ; ++it_l) {
			std::sort(it_l->second.begin(), it_l->second.end());
			lbs.push_back( it_l->first + "(" + boost::algorithm::join(it_l->second, ",") + ")" );
		}
		std::cerr << "* labels of functional expressions" << std::endl;
		std::cerr << boost::algorithm::join(lbs, " ") << std::endl;
	}
#endif


#if defined(USE_CRFSUITE)
	void tagger::train_crfsuite(const std::string &model_path, const std::vector< nlp::sentence > &tr_data, unsigned int freq) {
		boost::filesystem::path mp(model_path);
		train_crfsuite(mp, tr_data, freq);
	}


	void tagger::train_crfsuite(const boost::filesystem::path &mp, const std::vector< nlp::sentence > &tr_data, unsigned int freq) {
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
				gen_feat_crfsuite(tr_inst, tid_p, tid_pe, seq);
				for (int tid=tid_p ; tid<=tid_pe ; ++tid)
					yseq.push_back(tr_inst.get_token(tid)->fsem);
				crf_trainer.append(seq, yseq, 0);
			}
		}

		std::stringstream ss;
		ss << freq;
		crf_trainer.select("lbfgs", "crf1d");
		crf_trainer.set("minfreq", ss.str());
		BOOST_FOREACH (std::string name, crf_trainer.params())
			std::cout << name << "\t" << crf_trainer.get(name) << "\t" << crf_trainer.help(name) << std::endl;

		crf_trainer.train(model_path.string(), -1);
	}
#endif


#if defined(USE_CRFSUITE)
	/*
	 * * unigram:
	 *   表層、品詞、品詞細分類1、品詞細分類2、品詞細分類3、活用形
	 *   基本形、基本形+品詞、基本形+品詞細分類1、基本形+品詞細分類2
	 * * bigram:
	 *   品詞、表層、基本形、活用形+次の表層
	 */
	void tagger::gen_feat_crfsuite(nlp::sentence &sent, unsigned int tid_p, unsigned int tid_pe, CRFSuite::ItemSequence &seq) {
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
#endif


#if defined(USE_CRFSUITE)
	bool tagger::tag_by_crfsuite(nlp::sentence &sent, unsigned int tid_p, unsigned int tid_pe) {
#ifdef _MODEBUG
		std::vector<std::string> surfs;
		for (unsigned int tid=tid_p ; tid<=tid_pe ; ++tid)
			surfs.push_back(sent.get_token(tid)->surf);
		std::string surf = boost::algorithm::join(surfs, ".");
		std::cerr << "range of tokens for " << surf << " " << tid_p << ":" << tid_pe << " / " << sent.tid_min << ":" << sent.tid_max << std::endl;
#endif

		CRFSuite::ItemSequence seq;
		gen_feat_crfsuite(sent, tid_p, tid_pe, seq);

		CRFSuite::StringList list;
		list = fsem_tagger_crfs.tag(seq);
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
		if ((tok.pos == "動詞" && tok.pos1 == "接尾") || tok.pos1 == "非自立" || tok.pos == "助詞" || tok.pos == "助動詞" || std::binary_search(func_terms.begin(), func_terms.end(), tok.orig))
			return true;
		else
			return false;
	}


	bool tagger::is_pred(const nlp::token &tok, nlp::sentence &sent) {
		const nlp::token *t2 = sent.get_token(tok.id+1);
		if (tok.pos1 != "非自立" &&
			 	((tok.pos == "動詞" && tok.pos1 != "接尾") || tok.pos == "形容詞" || (tok.pos == "名詞" && tok.pos1 == "サ変接続") || (tok.pos=="名詞" && tok.pos1=="形容動詞語幹") || tok.pos == "副詞")
			 	&& !std::binary_search(func_terms.begin(), func_terms.end(), tok.orig) )
			return true;
		else if (tok.pos == "名詞" && t2 != NULL && (t2->pos1 == "接尾" || t2->pos == "助動詞"))
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


	void tagger::detect_target(nlp::sentence &sent, std::vector< std::vector< unsigned int > > &targets) {
		std::vector< unsigned int > tids_pred, tids_normal;
		BOOST_FOREACH (nlp::chunk chk, sent.chunks) {
			BOOST_FOREACH (nlp::token tok, chk.tokens) {
				if (is_pred(tok, sent))
					tids_pred.push_back(tok.id);
				else if (!is_func(tok))
					tids_normal.push_back(tok.id);
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
				for (unsigned int _tid=sent.tid_max ; _tid>=sent.tid_min ; --_tid) {
					if (sent.get_token(_tid)->pos != "記号") {
						_tids.push_back(_tid);
						break;
					}
				}
				if (_tids.size() != 2)
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

#if defined(USE_CRFPP)
//		set_feat_crfpp(sent);
#endif

		BOOST_FOREACH (std::vector< unsigned int > tids, targets) {
#if defined(USE_CRFPP)
			tag_by_crfpp(sent, tids[0], tids[1]);
#elif defined(USE_CRFSUITE)
			tag_by_crfsuite(sent, tids[0], tids[1]);
#endif
		}
	}
};

