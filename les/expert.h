#pragma once
#ifndef _LES_EXPERT_H_
#define _LES_EXPERT_H_

#include <stddef.h>

#include <les/knowbase.h>

typedef struct LittleExpertSystem
{
	KnowledgeBase kb;
	int nCurrentQuestion;
	double *currentProbs;

	double no, yes;
} LittleExpertSystem;

void les_init(LittleExpertSystem *pSys, const char *filename);
void les_close(LittleExpertSystem *pSys);

void les_start(LittleExpertSystem *pSys);
void les_answer(LittleExpertSystem *pSys, double answer);
void les_stop(LittleExpertSystem *pSys);

#endif
