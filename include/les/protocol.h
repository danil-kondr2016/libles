#pragma once
#ifndef _LES_PROTOCOL_H_
#define _LES_PROTOCOL_H_

#include <stddef.h>
#include <stdint.h>

#include <les/public.h>
#include <les/version.h>

typedef struct LESProtocol LESProtocol;
typedef struct LESProtocolNode LESProtocolNode;

struct LESProtocol
{
	LESProtocolNode *head;
	LESProtocolNode *tail;
};

struct LESProtocolNode
{
	LESProtocolNode *next;

	double          *probs;
	char            *question;

	double          normVal;

	int32_t         nHypotheses;
};

LIBLES_API
void les_protocol_init(LESProtocol *pProto);

LIBLES_API
void les_protocol_free(LESProtocol *pProto);

LIBLES_API
void les_protocol_move(LESProtocol *pDest, LESProtocol *pSrc);

LIBLES_API
void les_protocol_copy(LESProtocol *pDest, LESProtocol *pSrc);

LIBLES_API
void les_protocol_append(LESProtocol *proto,
		const char *question,
		double normVal,
		int32_t nHypotheses,
		double *probs);

#endif
