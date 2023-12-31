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

typedef struct KBParser KBParser;
typedef ptrdiff_t (*KBParserReadFn)(uintptr_t from, void *to, size_t size);

LIBLES_API
void les_knowledge_base_init_parser(KBParser *parser);

LIBLES_API
KBParser *les_knowledge_base_create_parser(
		uintptr_t input, KBParserReadFn fn);

LIBLES_API
void les_knowledge_base_destroy_parser(KBParser **pParser);

LIBLES_API
int les_knowledge_base_parse(KBParser *parser, KnowledgeBase *pKB);

LIBLES_API
KnowledgeBase *les_knowledge_base_create(void);

LIBLES_API
void les_knowledge_base_free(KnowledgeBase **pKB);

LIBLES_API
void les_knowledge_base_clear(KnowledgeBase *pKB);

LIBLES_API
void les_knowledge_base_copy(KnowledgeBase *pDest, KnowledgeBase *pSrc);

LIBLES_API
void les_knowledge_base_move(KnowledgeBase *pDest, KnowledgeBase *pSrc);

LES_DEF_END

#endif
