#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <les/expert.h>

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
	char *input_name, *question;
	FILE *input;
	KnowledgeBase kb = {0};
	KnowledgeBaseParser kbParser;
	LittleExpertSystem les = {0};
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
	printf("Result %d: %s\n", ret, les.kb.message);
	if (ret) {
		les_knowledge_base_destroy(&kb);
		fclose(input);
		return 1;
	}

	les_move_kb(&les, &kb);
	printf("%s\n", les.kb.comment);

	les_start(&les);
	while (les_is_running(&les)) {
		char *question;
		double x;	

		puts("Conclusions:");
		for (i = 0; i < les.kb.nConclusions; i++) {
			printf("  (%.5f) %s\n", 
					les.probs[i],
					les.kb.conclusions[i].str);
		}

		question = les_get_question(&les);
		printf("%s (-1=no, 1=yes): ", question);
		scanf(" %lf", &x);
		les_answer(&les, x);

		free(question);
	}
	puts("Conclusions:");
	for (i = 0; i < les.kb.nConclusions; i++) {
		printf("  (%.5f) %s\n", 
				les.probs[i],
				les.kb.conclusions[i].str);
	}


	les_close(&les);
	return 0;
}
