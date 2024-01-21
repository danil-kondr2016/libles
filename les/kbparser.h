#pragma once
#ifndef _KBPARSER_H_
#define _KBPARSER_H_

#include <les/knowbase.h>

enum KnowledgeBaseEncoding
{
	LES_KB_CP1251,
	LES_KB_UTF8,
};

typedef struct KBParser
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

	KBParserReadFn read;
	int encoding;
} KBParser;

#endif
