#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <les/knowbase.h>

static ptrdiff_t cfile_read(uintptr_t from, void *to, size_t size)
{
	ptrdiff_t result;
	size_t n_read;
	FILE *input;

	input = (FILE *)from;

	n_read = fread(to, 1, size, input);
	if (ferror(input))
		return -1;

	return n_read;
}

int main(int argc, char **argv)
{
	char *input_name;
	FILE *input;
	KnowledgeBase kb = {0};
	KnowledgeBaseParser kbParser = {0};
	int ret, i, j;

	if (argc < 2) {
		printf("Usage: %s input\n", argv[0]);
		exit(1);
	}

	input_name = argv[1];
	input = fopen(input_name, "rb");
	if (!input) {
		perror(input_name);
		return 1;
	}

	kbParser.input = (uintptr_t)input;
	kbParser.read = cfile_read;
	les_knowledge_base_init_parser(&kbParser);

	ret = les_knowledge_base_parse(&kbParser, &kb);
	printf("Result %d: %s\n", ret, kb.message);
	if (ret) {
		les_knowledge_base_destroy(&kb);
		return 1;
	}

	printf("Knowledge base comment:\nbegin\n%s\nend\n", kb.comment);

	printf("Number of questions: %d\n", kb.nQuestions - 1);
	for (i = 1; i < kb.nQuestions; i++) {
		printf("Question %d: %s\n", i, kb.questions[i]);
	}

	printf("Number of conclusions: %d\n", kb.nConclusions);
	for (i = 0; i < kb.nConclusions; i++) {
		printf("Conclusion: %d\n");
		printf("  Title: %s\n", kb.conclusions[i].str);
		printf("  Apriori probability: %lf\n", kb.conclusions[i].probApriori);
		printf("  Rules:\n");
		for (j = 1; j <= kb.nQuestions; j++) {
			printf("    Rule %d: py=%lf, pn=%lf\n",
					j, 
					kb.conclusions[i].answerProbs[j].probYes,
					kb.conclusions[i].answerProbs[j].probNo);
		}
	}

	les_knowledge_base_destroy(&kb);
	return 0;
}
