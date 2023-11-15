#include <les/expert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "buf.h"

static void select_question(LittleExpertSystem *pSys);
static void recalc_p_apriori(LittleExpertSystem *pSys, double answer);

void les_start(LittleExpertSystem *pSys)
{
	assert(pSys);

	pSys->running = 1;
	select_question(pSys);
}

int les_answer(LittleExpertSystem *pSys, double answer)
{
	double dunno, diff;
	double p, pe, py, pn;
	int i;

	assert(pSys);

	if (!pSys->running)
		return 0;

	if (answer < pSys->noVal)
		return 0;
	if (answer > pSys->yesVal)
		return 0;

	dunno = (pSys->yesVal + pSys->noVal) / 2.0;
	diff = (pSys->yesVal - dunno);
	answer = (answer - dunno) / diff;

	recalc_p_apriori(pSys, answer);
	select_question(pSys);
	return 1;
}

int les_is_running(LittleExpertSystem *pSys)
{
	assert(pSys);
	return pSys->running;
}

void les_stop(LittleExpertSystem *pSys)
{
	assert(pSys);

	pSys->running = 0;
	pSys->iCurrentQuestion = 0;
}

char *les_get_question(LittleExpertSystem *pSys)
{
	return strdup(pSys->kb.questions[pSys->iCurrentQuestion]);
}

static void select_question(LittleExpertSystem *pSys)
{
	double m = 0;
	size_t i, best_i;

	for (i = 1; i < pSys->kb.nQuestions; i++) {
		if (pSys->flags[i])
			continue;

		if (m < pSys->rulevalue[i]) {
			m = pSys->rulevalue[i];
			best_i = i;
		}
	}

	if (best_i == pSys->kb.nQuestions) {
		les_stop(pSys);
		return;
	}

	pSys->iCurrentQuestion = best_i;
	pSys->flags[best_i] = 1;
}

static void recalc_p_apriori(LittleExpertSystem *pSys, double answer)
{
	size_t i, q;
	double p, py, pn, pe;

	q = pSys->iCurrentQuestion;

	for (i = 0; i < pSys->kb.nConclusions; i++) {
		if (pSys->questions[i] == 0)
			continue;

		pSys->questions[i]--;

		p = pSys->probs[i];
		py = pSys->kb.conclusions[i].answerProbs[q].probYes;
		pn = pSys->kb.conclusions[i].answerProbs[q].probNo;
		pe = p * py + (1 - p) * pn;

		if (answer > 0) {
			p = p * (1 + (py/pe-1) * answer);
		}
		else {
			p = p * (1 + (py-(1-py)*pe/(1-pe)) * answer);
		}

		if (p < pSys->prob0 || p > pSys->prob1)
			pSys->questions[i] = 0;

		pSys->probs[i] = p;
	}
}
