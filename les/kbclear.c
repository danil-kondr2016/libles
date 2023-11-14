#include <les/knowbase.h>

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "buf.h"

void les_knowledge_base_destroy(KnowledgeBase *pKB)
{
	size_t i;

	if (pKB->comment) {
		free(pKB->comment);
		pKB->comment = NULL;
	}

	if (pKB->questions) {
		for (i = 0; i < pKB->nQuestions; i++)
			free(pKB->questions[i]);
		buf_free(pKB->questions);
		pKB->nQuestions = 0;
	}

	if (pKB->conclusions) {
		for (i = 0; i < pKB->nConclusions; i++) {
			if (pKB->conclusions[i].str)
				free(pKB->conclusions[i].str); 
			pKB->conclusions[i].str = NULL;
			pKB->conclusions[i].probApriori = 0;

			buf_free(pKB->conclusions[i].answerProbs);
			pKB->conclusions[i].nAnswerProbs = 0;
		}
		pKB->nConclusions = 0;
	}
}
