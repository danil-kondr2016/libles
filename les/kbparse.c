#include <les/knowbase.h>

#include <lib/stb/stb_ds.h>
#include <lib/sds/sds.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum kbparse_state
{
	COMMENT,
	QUESTION,
	CONCLUSION_TITLE,
	CONCLUSION_P_APRIORI,
	CONCLUSION_INDEX,
	CONCLUSION_PY,
	CONCLUSION_PN,
};

enum kbparse_error
{
	KBPARSE_ERROR_NOTHING,
	KBPARSE_ERROR_TOO_BIG,
	KBPARSE_ERROR_PREMATURE_END_OF_FILE,
	KBPARSE_ERROR_PREMATURE_END_OF_LINE,
};

struct read_buffer
{
	uint8_t   *buf;
	size_t    len;
	ptrdiff_t pos;
};

static int buf_getc(struct read_buffer *pBuf)
{
	if (!pBuf)
		return EOF;

	if (pBuf->pos >= pBuf->len)
		return EOF;

	return pBuf->buf[pBuf->pos++];
}

struct parse_context
{
	sds tmpString;
	void *input;
	KnowledgeBase *kb;
	Conclusion conc;
	AnswerProbability ansp;

	int nLines, lineLength, state;
	int nQuestions;
	int fragmentSize;

	int (*getc)(void *x);
};

static int parse(struct parse_context *ctx);

int les_knowledge_base_parse_file(KnowledgeBase *pKB, const char *filename)
{
	struct parse_context ctx = {0};
	FILE *input;

	input = fopen(filename, "rt");
	if (!input) {
		perror(filename);
		return -1;
	}

	ctx->input = input;
	ctx->getc = fgetc;
	ctx->kb = pKB;

	return parse(&ctx);
}

int les_knowledge_base_parse_data(KnowledgeBase *pKB, const char *data)
{
	struct parse_context ctx = {0};
	struct read_buffer rbuf = {0};

	rbuf.buf = data;
	rbuf.len = strlen(data);
	rbuf.pos = 0;

	ctx->input = &rbuf;
	ctx->getc = buf_getc;
	ctx->kb = pKB;

	return parse(&ctx);
}

static int parse(struct parse_context *ctx)
{
	char *input_name;
	FILE *input;
	int inc;
	int error = 0;
	char ch;

	ctx->tmpString = sdsempty();
	ctx->nLines = 0;
	ctx->lineLength = 0;
	ctx->state = 0;
	ctx->nQuestions = 0;
	ctx->fragmentSize = 0;

	ctx->kb->message = sdsempty();

	do {
		inc = ctx->getc(ctx->input);
		if (inc == EOF) {
			if (ctx->state != CONCLUSION_PN 
					&& (ctx->state != CONCLUSION_TITLE 
						|| ctx->lineLength > 0)) 
			{
				ctx->kb->message = sdscatprintf(ctx->kb->message,
						"Premature end of file at row %zd column %zd\n",
						ctx->nLines + 1,
						ctx->lineLength);
				error = KBPARSE_ERROR_PREMATURE_END_OF_FILE;
			}
		}

		if (inc != EOF)
			ch = inc;
		else
			ch = '\0';

		switch (ctx->state) {
		case COMMENT:
			if (inc == '\n') {
				if (ctx->lineLength == 0) {
					ctx->kb->comment = ctx->tmpString;
					ctx->tmpString = sdsempty();
					ctx->state++;
					continue;
				}
				else {
					ctx->lineLength = 0;
					ctx->nLines++;
				}
			}
			else {
				ctx->lineLength++;
			}

			ctx->tmpString = sdscatlen(ctx->tmpString, &ch, 1);
			break;
		case QUESTION:
			if (inc == '\n') {
				if (ctx->lineLength == 0) {
					ctx->state++;
					continue;
				} 
				else {
					arrput(ctx->kb->hypotheses, ctx->tmpString);

					ctx->lineLength = 0;
					ctx->nQuestions++;

					ctx->tmpString = sdsempty();
				}
			} 
			else {
				ctx->lineLength++;
				ctx->tmpString = sdscatlen(ctx->tmpString, &ch, 1);
			}
			break;
		case CONCLUSION_TITLE:
			if (inc == ',') {
				ctx->conc->str = ctx->tmpString;
				ctx->fragmentSize = 0;
				ctx->state++;
				ctx->tmpString = sdsempty();
				continue;
			}
		       	else if (inc == '\n') {
				if (ctx->lineLength > 0) {
					ctx->kb->message = sdscatprintf(ctx->message,
							"Premature end of line at row %zu column %zu",
							ctx->nLines + 1,
							ctx->lineLength);
					error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
					break;
				} else {
					ctx->lineLength = 0;
				}
			}
		       	else {
				ctx->fragmentSize++;
				ctx->lineLength++;
				ctx->tmpString = sdscatlen(ctx->tmpString, &ch, 1);
			}
			break;
		case CONCLUSION_P_APRIORI:
			if (inc == ',') {
				ctx->conc->probApriori = strtod(ctx->tmpString, NULL);
				sdsfree(ctx->tmpString);
				ctx->tmpString = sdsempty();

				ctx->fragmentSize = 0;
				ctx->state++;
				continue;
			}
		       	else if (inc == '\n') {
				ctx->kb->message = sdscatprintf(ctx->message,
						"Premature end of line at row %zu column %zu",
						ctx->nLines + 1,
						ctx->lineLength);
				error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
				break;
			}
		       	else {
				ctx->fragmentSize++;
				ctx->lineLength++;
				ctx->tmpString = sdscatlen(ctx->tmpString, &ch, 1);
			}
			break;
		case CONCLUSION_INDEX:
			if (inc == ',') {
				ctx->ansp->iHypothesis = atoi(ctx->tmpString);
				sdsfree(ctx->tmpString);
				ctx->tmpString = sdsempty();

				ctx->fragmentSize = 0;
				ctx->state++;
				continue;
			}
		       	else if (inc == '\n') {
				ctx->kb->message = sdscatprintf(ctx->message,
						"Premature end of line at row %zu column %zu",
						ctx->nLines + 1,
						ctx->lineLength);
				error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
				break;
			}
		       	else {
				ctx->fragmentSize++;
				ctx->lineLength++;
				ctx->tmpString = sdscatlen(ctx->tmpString, &ch, 1);
			}
			break;
		case CONCLUSION_PY:
			if (inc == ',') {
				ctx->ansp->probYes = strtod(ctx->tmpString, NULL);
				sdsfree(ctx->tmpString);
				ctx->tmpString = sdsempty();

				ctx->fragmentSize = 0;
				ctx->state++;
				continue;
			}
		       	else if (inc == '\n') {
				ctx->kb->message = sdscatprintf(ctx->message,
						"Premature end of line at row %zu column %zu",
						ctx->nLines + 1,
						ctx->lineLength);
				error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
				break;
			}
		       	else {
				ctx->fragmentSize++;
				ctx->lineLength++;
				ctx->tmpString = sdscatlen(ctx->tmpString, &ch, 1);
			}
			break;
		case CONCLUSION_PN:
			if (inc == ',') {
				ctx->ansp->probNo = strtod(ctx->tmpString, NULL);
				sdsfree(ctx->tmpString);
				ctx->tmpString = sdsempty();

				arrput(ctx->conc->answerProbs, ctx->ansp);
				ctx->ansp->iHypothesis = 0;
				ctx->ansp->probYes = 0;
				ctx->ansp->probNo = 0;

				ctx->fragmentSize = 0;
				ctx->state = CONCLUSION_INDEX;
				continue;
			}
		       	else if (inc == '\n') {
				if (ctx->fragmentSize == 0) {
					ctx->kb->message = sdscatprintf(ctx->message,
							"Premature end of line at row %zu column %zu",
							ctx->nLines + 1,
							ctx->lineLength);
					error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
					break;
				}
				else {
					ctx->ansp->probNo = strtod(ctx->tmpString, NULL);
					sdsfree(ctx->tmpString);
					ctx->tmpString = sdsempty();

					arrput(ctx->conc->answerProbs, ctx->ansp);
					ctx->ansp->iHypothesis = 0;
					ctx->ansp->probYes = 0;
					ctx->ansp->probNo = 0;

					ctx->conc->nAnswerProbs = arrlenu(ctx->conc->answerProbs);
					arrput(ctx->kb->conclusions, ctx->conc);

					ctx->fragmentSize = 0;
					ctx->lineLength = 0;
					
					ctx->state = CONCLUSION_TITLE;
					continue;
				}
			}
		       	else {
				ctx->fragmentSize++;
				ctx->lineLength++;
				ctx->tmpString = sdscatlen(ctx->tmpString, &ch, 1);
			}
			break;
		}
	} while (inc != EOF && !error);

	return error;
}
