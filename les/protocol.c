#define _LES_DLL
#include <les/protocol.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "buf.h"

LIBLES_API
void les_protocol_init(LESProtocol *pProto)
{
	assert(pProto);

	pProto->head = NULL;
	pProto->tail = NULL;
}

LIBLES_API
void les_protocol_free(LESProtocol *pProto)
{
	LESProtocolNode *p, *q;

	assert(pProto);

	p = pProto->head;
	while (p) {
		q = p;
		p = p->next;

		buf_free(q->probs);
		free(q->question);
		free(q);
	}

	pProto->head = NULL;
	pProto->tail = NULL;
}

LIBLES_API
void les_protocol_move(LESProtocol *pDest, LESProtocol *pSrc)
{
	assert(pDest);
	assert(pSrc);

	pDest->head = pSrc->head;
	pDest->tail = pSrc->tail;

	pSrc->head = NULL;
	pSrc->tail = NULL;
}

LIBLES_API
void les_protocol_copy(LESProtocol *pDest, LESProtocol *pSrc)
{
	LESProtocolNode *p;

	assert(pDest);
	assert(pSrc);

	les_protocol_init(pDest);
	p = pSrc->head;
	while (p) {
		les_protocol_append(pDest, 
				p->question,
				p->normVal,
				p->nHypotheses,
				p->probs);
		p = p->next;
	}
}

LIBLES_API
void les_protocol_append(LESProtocol *proto,
		const char *question,
		double normVal,
		int nHypotheses,
		double *probs)
{
	LESProtocolNode *pNewNode;
	int i;

	assert(proto);

	pNewNode = calloc(1, sizeof(LESProtocolNode));
	pNewNode->question = strdup(question);
	pNewNode->normVal = normVal;
	pNewNode->nHypotheses = nHypotheses;
	pNewNode->probs = NULL;

	for (i = 0; i < nHypotheses; i++)
		buf_push(pNewNode->probs, probs[i]);

	if (!proto->head && !proto->tail) {
		proto->head = pNewNode;
		proto->tail = pNewNode;
	}
	else {
		proto->tail->next = pNewNode;
		proto->tail = proto->tail->next;
	}
}
