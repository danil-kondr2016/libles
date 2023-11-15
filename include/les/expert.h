#pragma once
#ifndef _LES_EXPERT_H_
#define _LES_EXPERT_H_

#include <stddef.h>

#include <les/knowbase.h>

typedef struct LittleExpertSystem
{
	KnowledgeBase kb;

	double       *probs;
	double       *rulevalue;
	int          *questions;
	uint8_t      *flags;
	double        yesVal, noVal;
	double        prob0, prob1;
	int           iCurrentQuestion;
	int           running;
} LittleExpertSystem;

void les_init_file(LittleExpertSystem *pSys, const char *filename);
void les_init_data(LittleExpertSystem *pSys, const char *data);
void les_close(LittleExpertSystem *pSys);

void les_start(LittleExpertSystem *pSys);
int  les_is_running(LittleExpertSystem *pSys);
int  les_answer(LittleExpertSystem *pSys, double answer);
void les_stop(LittleExpertSystem *pSys);

#endif
