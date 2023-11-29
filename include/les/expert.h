#pragma once
#ifndef _LES_EXPERT_H_
#define _LES_EXPERT_H_

#include <stddef.h>

#include <les/version.h>
#include <les/public.h>
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
	int32_t       iCurrentQuestion;
	int32_t       iBest;

	int           running;
} LittleExpertSystem;

LIBLES_API
void les_move_kb(LittleExpertSystem *pSys, KnowledgeBase *pKB);

LIBLES_API
void les_copy_kb(LittleExpertSystem *pSys, KnowledgeBase *pKB);

LIBLES_API
void les_close(LittleExpertSystem *pSys);

LIBLES_API
void les_start(LittleExpertSystem *pSys);

LIBLES_API
int  les_is_running(LittleExpertSystem *pSys);

LIBLES_API
int  les_answer(LittleExpertSystem *pSys, double answer);

LIBLES_API
void les_stop(LittleExpertSystem *pSys);

LIBLES_API
char *les_get_question(LittleExpertSystem *pSys);

#endif
