#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <les/knowbase.h>

int main(int argc, char **argv)
{
	char *input_name;
	KnowledgeBase kb = {0};
	int ret, i, j;

	if (argc < 2) {
		printf("Usage: %s input\n", argv[0]);
		exit(1);
	}

	input_name = argv[1];
	ret = les_knowledge_base_parse_file(&kb, input_name);
	printf("Result %d: %s\n", ret, kb.message);
	if (ret) {
		les_knowledge_base_destroy(&kb);
		return 1;
	}

	printf("Knowledge base comment:\nbegin\n%s\nend\n", kb.comment);

	printf("Number of questions: %d\n", kb.nQuestions);
	for (i = 0; i < kb.nQuestions; i++) {
		printf("Question %d: %s\n", i, kb.questions[i]);
	}

	printf("Number of conclusions: %d\n", kb.nConclusions);
	for (i = 0; i < kb.nConclusions; i++) {
		printf("Conclusion: %d\n");
		printf("  Title: %s\n", kb.conclusions[i].str);
		printf("  Apriori probability: %lf\n", kb.conclusions[i].probApriori);
		printf("  Number of rules: %d\n", kb.conclusions[i].nAnswerProbs);
		printf("  Rules:\n");
		for (j = 0; j < kb.conclusions[i].nAnswerProbs; j++) {
			printf("    Rule %d: i=%d, py=%lf, pn=%lf\n",
					j, 
					kb.conclusions[i].answerProbs[j].iHypothesis,
					kb.conclusions[i].answerProbs[j].probYes,
					kb.conclusions[i].answerProbs[j].probNo);
		}
	}

	les_knowledge_base_destroy(&kb);
	return 0;
}
