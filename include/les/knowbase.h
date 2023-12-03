#pragma once
#ifndef _LES_KNOWBASE_H_
#define _LES_KNOWBASE_H_

#include <stdint.h>
#include <stddef.h>

#include <les/public.h>
#include <les/version.h>

LES_DEF_BEGIN

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

	char            **questions;
	Conclusion      *conclusions;

	int32_t         nQuestions;
	int32_t         nConclusions;
};
typedef struct KnowledgeBase KnowledgeBase;

typedef struct KnowledgeBaseParser
{
	char *tmpBuf;
	uintptr_t input;
	KnowledgeBase *kb;
	Conclusion conc;
	AnswerProbability ansp;

	int nLines, lineLength, state;
	int nQuestions;
	int fragmentSize;
	int error;

	int iAnswerProbQuestion;

	ptrdiff_t (*read)(uintptr_t from, void *to, size_t size);
} KnowledgeBaseParser;

LIBLES_API
void les_knowledge_base_init_parser(KnowledgeBaseParser *pParser);

LIBLES_API
int les_knowledge_base_parse(KnowledgeBaseParser *pParser, KnowledgeBase *pKB);

LIBLES_API
void les_knowledge_base_destroy(KnowledgeBase *pKB);

LIBLES_API
void les_knowledge_base_copy(KnowledgeBase *pDest, KnowledgeBase *pSrc);

LIBLES_API
void les_knowledge_base_move(KnowledgeBase *pDest, KnowledgeBase *pSrc);

LES_DEF_END

#endif
