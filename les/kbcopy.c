#include <les/knowbase.h>

#include <string.h>
#include "buf.h"

void les_knowledge_base_copy(KnowledgeBase *pDest, KnowledgeBase *pSrc)
{
	size_t i, j;
	Conclusion conclusion;

	strncpy(pDest->message, pSrc->message, MAX_MESSAGE_LENGTH + 1);
	pDest->comment = strdup(pSrc->comment);

	pDest->nQuestions = pSrc->nQuestions;
	pDest->questions = NULL;
	for (i = 0; i < pDest->nQuestions; i++) {
		buf_push(pDest->questions, strdup(pSrc->questions[i]));
	}

	pDest->nConclusions = pSrc->nConclusions;
	pDest->conclusions = NULL;
	for (i = 0; i < pDest->nConclusions; i++) {
		conclusion.str = strdup(pSrc->conclusions[i].str);
		conclusion.probApriori = pSrc->conclusions[i].probApriori;
		conclusion.answerProbs = NULL;
		
		for (j = 0; j < pDest->nQuestions; j++) {
			buf_push(conclusion.answerProbs,
					pSrc->conclusions[i].answerProbs[j]);
		}

		buf_push(pDest->conclusions, conclusion);
	}
}
