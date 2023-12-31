#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <les/expert.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#define LESRUN_VERSION "0.2.0"

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
	KBParser *kbParser;
	LittleExpertSystem les = {0};
	struct optparse options;
	int ret, i, j, option;

	optparse_init(&options, argv);

	while  ((option = optparse(&options, "hv")) != -1) {
		switch (option) {
		case 'h':
			printf("Usage: %s [-h] input\n", argv[0]);
			exit(EXIT_SUCCESS);
		case 'v':
			printf("lesrun %s\n", LESRUN_VERSION);
			printf("based on libles %s\n", les_version());
			exit(EXIT_SUCCESS);
		case '?':
			fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
			exit(EXIT_FAILURE);
		}
	}

	input_name = optparse_arg(&options);
	if (!input_name) {
		printf("%s: input name not specified\n", argv[0]);
		printf("Usage: %s [-h] input\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input = fopen(input_name, "rb");
	if (!input) {
		perror(input_name);
		return 1;
	}

	kbParser = les_knowledge_base_create_parser(
			(uintptr_t)input, cfile_read);

	ret = les_knowledge_base_parse(kbParser, &kb);
	printf("Result %d: %s\n", ret, kb.message);
	if (ret) {
		les_knowledge_base_clear(&kb);
		fclose(input);
		return 1;
	}

	les_knowledge_base_destroy_parser(&kbParser);

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
