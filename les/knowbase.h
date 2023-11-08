#pragma once
#ifndef _LES_KNOWBASE_H_
#define _LES_KNOWBASE_H_

#include <stdint.h>
#include <stddef.h>

struct AnswerProbability
{
	int     iHypothesis;
	double  probYes, probNo;
};
typedef struct AnswerProbability AnswerProbability;

struct Conclusion
{
	char               *str;
	double             probApriori;

	size_t             nAnswerProbs;
	AnswerProbability *answerProbs;
};
typedef struct Conclusion Conclusion;

struct KnowledgeBase
{
	char            *comment;

	size_t          nHypotheses;
	char            **hypotheses;

	size_t          nConclusions;
	Conclusion      *conclusions;
};
typedef struct KnowledgeBase KnowledgeBase;

int les_knowledge_base_parse_file(KnowledgeBase *pKB, const char *filename);
int les_knowledge_base_parse_data(KnowledgeBase *pKB, const char *data);
void les_knowledge_base_destroy(KnowledgeBase *pKB);

#endif
