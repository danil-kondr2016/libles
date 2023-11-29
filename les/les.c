#define _LES_DLL
#include <les/expert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "buf.h"

static void select_question(LittleExpertSystem *pSys);
static void recalc_p_apriori(LittleExpertSystem *pSys, double answer);
static void calculate_rulevalue(LittleExpertSystem *pSys);
static void set_rulevalue(LittleExpertSystem *pSys, int32_t i);
static void calculate_min_max(LittleExpertSystem *pSys);

LIBLES_API
void les_start(LittleExpertSystem *pSys)
{
	assert(pSys);

	pSys->running = 1;
	select_question(pSys);
}

LIBLES_API
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
	calculate_min_max(pSys);
	if (!les_is_running(pSys))
		return 1;

	select_question(pSys);
	return 1;
}

LIBLES_API
int les_is_running(LittleExpertSystem *pSys)
{
	assert(pSys);
	return pSys->running;
}

LIBLES_API
void les_stop(LittleExpertSystem *pSys)
{
	assert(pSys);

	pSys->running = 0;
	pSys->iCurrentQuestion = 0;
}

LIBLES_API
char *les_get_question(LittleExpertSystem *pSys)
{
	return strdup(pSys->kb.questions[pSys->iCurrentQuestion]);
}

static void calculate_rulevalue(LittleExpertSystem *pSys)
{
	int32_t i;

	for (i = 0; i < pSys->kb.nConclusions; i++)
		set_rulevalue(pSys, i);
}

static void set_rulevalue(LittleExpertSystem *pSys, int32_t i)
{
	double p, py, pn, p_if_y, p_if_not_y;
	int32_t j;

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
		}
	}
}


static void select_question(LittleExpertSystem *pSys)
{
	double m = 0;
	int32_t i, best_i;

	calculate_rulevalue(pSys);

	for (i = 1; i < pSys->kb.nQuestions; i++) {
		if (!pSys->flags[i])
			continue;

		if (m < pSys->rulevalue[i]) {
			m = pSys->rulevalue[i];
			best_i = i;
		}
	}

	if (best_i >= pSys->kb.nQuestions) {
		les_stop(pSys);
		return;
	}

	pSys->iCurrentQuestion = best_i;
	pSys->flags[best_i] = 0;
}

static void recalc_p_apriori(LittleExpertSystem *pSys, double answer)
{
	int32_t i, q;
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

static void calculate_min_max(LittleExpertSystem *pSys)
{
	double maxofmin, prior, a1, a2, a3, a4, p, py, pn;
	int32_t i, j, best;

	maxofmin = 0;
	for (i = 0; i < pSys->kb.nConclusions; i++) {
		p = pSys->probs[i];
		prior = pSys->kb.conclusions[i].probApriori;

		a1 = 1;
		a2 = 1;
		a3 = 1;
		a4 = 1;

		for (j = 1; j < pSys->kb.nQuestions; j++) {
			py = pSys->kb.conclusions[i].answerProbs[j].probYes;
			pn = pSys->kb.conclusions[i].answerProbs[j].probNo;

			if (pSys->flags[i]*pSys->questions[i] == 0)
				continue;

			if (pn > py) {
				pn = 1 - pn;
				py = 1 - py;
			}

			a1 *= py;
			a2 *= pn;
			a3 *= (1 - py);
			a4 *= (1 - pn);
		}

		pSys->max[i] = p*a1/(p*a1+(1-p)*a2);
		pSys->min[i] = p*a3/(p*a3+(1-p)*a4);
		if (pSys->max[i] < prior) {
			pSys->questions[i] = 0;
			/* Можно исключить pSys->kb.questions[i] */
		}
		if (pSys->min[i] > maxofmin) {
			best = i;
			maxofmin = pSys->min[i];
		}
	}

	pSys->iBest = best;
	for (i = 0; i < pSys->kb.nConclusions; i++) {
		if (pSys->min[best] <= pSys->max[i])
			maxofmin = 0;
	}

	if (maxofmin != 0) {
		les_stop(pSys);
	}
}
