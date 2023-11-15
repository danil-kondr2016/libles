#include <les/expert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

static void set_rulevalue_questions(LittleExpertSystem *pSys, size_t i);
static void les_init_fields(LittleExpertSystem *pSys);

int les_init_file(LittleExpertSystem *pSys, const char *filename)
{
	int ret;
	assert(pSys);

	ret = les_knowledge_base_parse_file(&pSys->kb, filename);
	if (ret)
		return ret;

	les_init_fields(pSys);
	return ret;
}

int les_init_data(LittleExpertSystem *pSys, const char *data)
{
	int ret;
	assert(pSys);

	ret = les_knowledge_base_parse_data(&pSys->kb, data);
	if (ret)
		return ret;

	les_init_fields(pSys);
	return ret;
}

void les_close(LittleExpertSystem *pSys)
{
	assert(pSys);

	les_knowledge_base_destroy(&pSys->kb);	
	free(pSys->probs);
	free(pSys->rulevalue);
	free(pSys->questions);
	free(pSys->flags);
}

static void set_rulevalue_questions(LittleExpertSystem *pSys, size_t i)
{
	double p, py, pn, p_if_y, p_if_not_y;
	size_t j;

	p = pSys->kb.conclusions[i].probApriori;
	pSys->rulevalue[i] = 0;

	for (j = 1; j < pSys->kb.nQuestions; j++) {
		double rv_incr;
		py = pSys->kb.conclusions[i].answerProbs[j].probYes;
		pn = pSys->kb.conclusions[i].answerProbs[j].probNo;
		if (py == 0.5 && pn == 0.5) {
			continue;
		}
		else {
			p_if_y = p*py / (py*p + pn*(1-p));
			p_if_not_y = p*(1-py) / ((1-py)*p + (1-pn)*(1-p));

			rv_incr = p_if_y - p_if_not_y;
			if (rv_incr < 0)
				rv_incr = -rv_incr;

			pSys->rulevalue[i] += rv_incr;
			pSys->questions[i]++;
		}
	}
}

static void les_init_fields(LittleExpertSystem *pSys)
{
	size_t i;
	
	pSys->probs = calloc(pSys->kb.nConclusions, sizeof(double));
	pSys->rulevalue = calloc(pSys->kb.nConclusions, sizeof(double));
	pSys->questions = calloc(pSys->kb.nConclusions, sizeof(int));
	pSys->flags = calloc(pSys->kb.nConclusions, sizeof(uint8_t));

	for (i = 0; i < pSys->kb.nConclusions; i++) {
		pSys->probs[i] = pSys->kb.conclusions[i].probApriori;
		set_rulevalue_questions(pSys, i);
	}

	pSys->yesVal = 1;
	pSys->noVal = -1;
	pSys->prob0 = 0.00001;
	pSys->prob1 = 0.99999;
	pSys->iCurrentQuestion = 0;
}


