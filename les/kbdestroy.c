#include <les/knowbase.h>

#include <lib/stb/stb_ds.h>
#include <lib/sds/sds.h>

#include <stdio.h>
#include <stdlib.h>

void les_knowledge_base_destroy(KnowledgeBase *pKB)
{
	size_t i;

	if (pKB->message) {
		sdsfree(pKB->message);
		pKB->message = NULL;
	}

	if (pKB->comment) {
		sdsfree(pKB->comment);
		pKB->comment = NULL;
	}

	if (pKB->questions) {
		for (i = 0; i < arrlen(pKB->questions); i++)
			sdsfree(pKB->questions[i]);
		arrfree(pKB->questions);
		pKB->nQuestions = 0;
	}

	if (pKB->conclusions) {
		for (i = 0; i < pKB->nConclusions; i++) {

			if (pKB->conclusions[i].str)
				sdsfree(pKB->conclusions[i].str); 
			pKB->conclusions[i].str = NULL;
			pKB->conclusions[i].probApriori = 0;

			arrfree(pKB->conclusions[i].answerProbs);
			pKB->conclusions[i].nAnswerProbs = 0;
		}
		pKB->nConclusions = 0;
	}
}
