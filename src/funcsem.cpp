#include <iostream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <crfsuite.hpp>
//#include <crfsuite_api.hpp>
#include "modality.hpp"
#include "sentence.hpp"

namespace funcsem {
	bool tagger::tag_by_crf(std::vector<nlp::token *> &toks) {
	}

	bool is_pred(nlp::token *tok) {
		if (tok->pos == "動詞" || tok->pos == "形容詞" || (tok->pos == "名詞" && tok->pos1 == "サ変接続") || (tok->pos=="名詞" && tok->pos1=="形容動詞語幹"))
			return true;
		else
			return false;
	}

	void tagger::tag(nlp::sentence &sent) {
		std::vector<nlp::token *> toks;
		BOOST_REVERSE_FOREACH (nlp::chunk chk, sent.chunks) {
			std::vector<nlp::token>::reverse_iterator rit_tok, rite_tok=chk.tokens.rend();
			for (rit_tok=chk.tokens.rbegin() ; rit_tok!=rite_tok ; ++rit_tok) {
				toks.push_back(&(*rit_tok));
			}
		}

		std::reverse(toks.begin(), toks.end());
		unsigned int i_tok;
		std::vector<nlp::token *> toks_tagged;
		bool to_be_tagged = false;
		for (i_tok=0 ; i_tok<toks.size() ; ++i_tok) {
			if (toks[i_tok]->pos == "記号") {
				continue;
			}

			if (to_be_tagged == false && is_pred(toks[i_tok])) {
				to_be_tagged = true;
			}
			else if (to_be_tagged && (std::binary_search(func_terms.begin(), func_terms.end(), toks[i_tok]->orig) || !is_pred(toks[i_tok])) ) {
				toks_tagged.push_back(toks[i_tok]);
			}
			else {
				BOOST_FOREACH (nlp::token *t, toks_tagged) {
					std::cout << t->surf << " ";
				}
				std::cout << std::endl;
				to_be_tagged = false;
				toks_tagged.clear();
			}

			toks_tagged.push_back(toks[i_tok]);
		}
	}
};

