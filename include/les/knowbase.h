#pragma once
#ifndef _LES_KNOWBASE_H_
#define _LES_KNOWBASE_H_

#include <stdint.h>
#include <stddef.h>

struct AnswerProbability
{
	double  probYes, probNo;
};
typedef struct AnswerProbability AnswerProbability;

struct Conclusion
{
	char               *str;
	double             probApriori;

	AnswerProbability  *answerProbs;
};
typedef struct Conclusion Conclusion;

#define MAX_MESSAGE_LENGTH 127

struct KnowledgeBase
{
	char            message[MAX_MESSAGE_LENGTH + 1];
	char            *comment;

	size_t          nQuestions;
	char            **questions;

	size_t          nConclusions;
	Conclusion      *conclusions;
};
typedef struct KnowledgeBase KnowledgeBase;

int les_knowledge_base_parse_file(KnowledgeBase *pKB, const char *filename);
int les_knowledge_base_parse_data(KnowledgeBase *pKB, const char *data);
void les_knowledge_base_destroy(KnowledgeBase *pKB);

#endif
