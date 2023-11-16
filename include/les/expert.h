#pragma once
#ifndef _LES_EXPERT_H_
#define _LES_EXPERT_H_

#include <stddef.h>

#include <les/knowbase.h>

typedef struct LittleExpertSystem
{
	KnowledgeBase kb;

	double       *probs;
	double       *min;
	double       *max;
	double       *rulevalue;
	int          *questions;
	uint8_t      *flags;

	double        yesVal, noVal;
	double        prob0, prob1;
	size_t        iCurrentQuestion;
	size_t        iBest;

	int           running;
} LittleExpertSystem;

void les_move_kb(LittleExpertSystem *pSys, KnowledgeBase *pKB);
void les_close(LittleExpertSystem *pSys);

void les_start(LittleExpertSystem *pSys);
int  les_is_running(LittleExpertSystem *pSys);
int  les_answer(LittleExpertSystem *pSys, double answer);
void les_stop(LittleExpertSystem *pSys);

char *les_get_question(LittleExpertSystem *pSys);

#endif
