#include <les/expert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

static void set_questions(LittleExpertSystem *pSys, int32_t i);
static void init_fields(LittleExpertSystem *pSys);

LIBLES_API
void les_move_kb(LittleExpertSystem *pSys, KnowledgeBase *pKB)
{
	assert(pSys);
	assert(pKB);

	les_knowledge_base_move(&pSys->kb, pKB);
	init_fields(pSys);
}

LIBLES_API
void les_copy_kb(LittleExpertSystem *pSys, KnowledgeBase *pKB)
{
	assert(pSys);
	assert(pKB);

	les_knowledge_base_copy(&pSys->kb, pKB);
	init_fields(pSys);
}

LIBLES_API
void les_close(LittleExpertSystem *pSys)
{
	assert(pSys);

	les_knowledge_base_clear(&pSys->kb);	
	free(pSys->probs);
	free(pSys->min);
	free(pSys->max);
	free(pSys->rulevalue);
	free(pSys->questions);
	free(pSys->flags);
}

static void set_questions(LittleExpertSystem *pSys, int32_t i)
{
	double p, py, pn;
	int32_t j;

	p = pSys->kb.conclusions[i].probApriori;
	pSys->questions[i] = 0;

	for (j = 1; j < pSys->kb.nQuestions; j++) {
		double rv_incr;
		py = pSys->kb.conclusions[i].answerProbs[j].probYes;
		pn = pSys->kb.conclusions[i].answerProbs[j].probNo;
		if (py != 0.5 || pn != 0.5) {
			pSys->questions[i]++;
		}
	}
}

static void init_fields(LittleExpertSystem *pSys)
{
	int32_t i;
	
	pSys->probs = calloc(pSys->kb.nConclusions, sizeof(double));
	pSys->min = calloc(pSys->kb.nConclusions, sizeof(double));
	pSys->max = calloc(pSys->kb.nConclusions, sizeof(double));
	pSys->rulevalue = calloc(pSys->kb.nQuestions, sizeof(double));
	pSys->questions = calloc(pSys->kb.nConclusions, sizeof(int));
	pSys->flags = calloc(pSys->kb.nQuestions, sizeof(uint8_t));

	for (i = 0; i < pSys->kb.nConclusions; i++) {
		pSys->probs[i] = pSys->kb.conclusions[i].probApriori;
		set_questions(pSys, i);
	}

	for (i = 0; i < pSys->kb.nQuestions; i++) {
		pSys->flags[i] = 1;
	}

	pSys->yesVal = 1;
	pSys->noVal = -1;
	pSys->prob0 = 0.00001;
	pSys->prob1 = 0.99999;
	pSys->iCurrentQuestion = 0;
}
