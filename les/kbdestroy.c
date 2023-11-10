#include <les/knowbase.h>

#include <stb/stb_ds.h>
#include <sds/sds.h>

#include <stdio.h>
#include <stdlib.h>

void les_knowledge_base_destroy(KnowledgeBase *pKB)
{
	size_t i;

	sdsfree(pKB->comment);
	pKB->comment = NULL;

	if (pKB->hypotheses) {
		for (i = 0; i < arrlen(pKB->hypotheses); i++)
			sdsfree(pKB->hypotheses[i]);
		arrfree(pKB->hypotheses);
		pKB->nHypotheses = 0;
	}

	if (pKB->conclusions) {
		for (i = 0; i < pKB->nConclusions; i++) {

			if (pKB->conclusions[i].str)
				sdsfree(pKB->conclusions[i].str); 
			pKB->conclusions[i].str = NULL;
			pKB->conclusions[i].probApriori = 0;

			pKB->conclusions[i].nAnswerProbs = 0;
			arrfree(pKB->conclusions[i].answerProbs);
		}
		pKB->nConclusions = 0;
	}
}
