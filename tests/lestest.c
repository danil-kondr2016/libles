#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <les/expert.h>

int main(int argc, char **argv)
{
	char *input_name, *question;
	LittleExpertSystem les = {0};
	int ret, i, j;

	if (argc < 2) {
		printf("Usage: %s input\n", argv[0]);
		exit(1);
	}

	input_name = argv[1];
	ret = les_init_file(&les, input_name);
	printf("Result %d: %s\n", ret, les.kb.message);
	if (ret) {
		les_close(&les);
		return 1;
	}


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
				les.kb.conclusions[i]);
	}


	les_close(&les);
	return 0;
}
