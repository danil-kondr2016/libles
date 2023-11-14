#include <les/knowbase.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <errno.h>

#include "buf.h"

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
	char *tmpBuf;
	void *input;
	KnowledgeBase *kb;
	Conclusion conc;
	AnswerProbability ansp;

	int nLines, lineLength, state;
	int nQuestions;
	int fragmentSize;
	int error;

	int iAnswerProbQuestion;

	int (*getc)(void *x);
};

static int parse(struct parse_context *ctx);

int les_knowledge_base_parse_file(KnowledgeBase *pKB, const char *filename)
{
	struct parse_context ctx = {0};
	FILE *input;

	input = fopen(filename, "rt");
	if (!input) {
		int _error = errno;
		snprintf(pKB->message, MAX_MESSAGE_LENGTH,
				"System error: %s: %s",
				filename, strerror(_error));
		return -1;
	}

	ctx.input = input;
	ctx.getc = fgetc;
	ctx.kb = pKB;

	return parse(&ctx);
}

int les_knowledge_base_parse_data(KnowledgeBase *pKB, const char *data)
{
	struct parse_context ctx = {0};
	struct read_buffer rbuf = {0};

	rbuf.buf = data;
	rbuf.len = strlen(data);
	rbuf.pos = 0;

	ctx.input = &rbuf;
	ctx.getc = buf_getc;
	ctx.kb = pKB;

	return parse(&ctx);
}

enum
{
	CONTINUE,
	STOP,
};

static int parse_comment(struct parse_context *ctx, int inc);
static int parse_question(struct parse_context *ctx, int inc);
static int parse_conc_title(struct parse_context *ctx, int inc);
static int parse_conc_p_apriori(struct parse_context *ctx, int inc);
static int parse_conc_rule_index(struct parse_context *ctx, int inc);
static int parse_conc_rule_py(struct parse_context *ctx, int inc);
static int parse_conc_rule_pn(struct parse_context *ctx, int inc);

static void init_conclusion(Conclusion *conc, 
		int nQuestions)
{
	int i;
	AnswerProbability apIndifferent = {0.5, 0.5};

	assert(conc);

	conc->str = NULL;
	conc->probApriori = 0;
	conc->answerProbs = NULL;

	for (i = 0; i < nQuestions+1; i++)
		buf_push(conc->answerProbs, apIndifferent);
}

static int parse(struct parse_context *ctx)
{
	int inc;
	int ret;
	char ch;

	ctx->nLines = 0;
	ctx->lineLength = 0;
	ctx->state = 0;
	ctx->nQuestions = 0;
	ctx->fragmentSize = 0;

	do {
		inc = ctx->getc(ctx->input);
		if (inc == EOF) {
			if (ctx->state != CONCLUSION_PN 
					&& (ctx->state != CONCLUSION_TITLE 
						|| ctx->lineLength > 0)) 
			{
				snprintf(ctx->kb->message, MAX_MESSAGE_LENGTH,
					"Premature end of file at row %zd column %zd\n",
					ctx->nLines + 1,
					ctx->lineLength);
				ctx->error = KBPARSE_ERROR_PREMATURE_END_OF_FILE;
				break;
			}
		}

		if (inc != EOF)
			ch = inc;
		else
			ch = '\0';

		switch (ctx->state) {
		case COMMENT:
			ret = parse_comment(ctx, inc);
			break;
		case QUESTION:
			ret = parse_question(ctx, inc);
			break;
		case CONCLUSION_TITLE:
			ret = parse_conc_title(ctx, inc);
			break;
		case CONCLUSION_P_APRIORI:
			ret = parse_conc_p_apriori(ctx, inc);
			break;
		case CONCLUSION_INDEX:
			ret = parse_conc_rule_index(ctx, inc);
			break;
		case CONCLUSION_PY:
			ret = parse_conc_rule_py(ctx, inc);
			break;
		case CONCLUSION_PN:
			ret = parse_conc_rule_pn(ctx, inc);
			break;
		}

		if (ret == STOP)
			break;
	} while (inc != EOF && !ctx->error);

	buf_free(ctx->tmpBuf);

	return ctx->error;
}

static int parse_comment(struct parse_context *ctx, int inc)
{
	char ch;

	assert(ctx->state == COMMENT);

	ch = inc;
	if (inc == '\n') {
		if (ctx->lineLength == 0) {
			buf_push(ctx->tmpBuf, '\0');
			ctx->kb->comment = strdup(ctx->tmpBuf);
			buf_clear(ctx->tmpBuf);
			ctx->state++;
			return CONTINUE;
		}
		else {
			ctx->lineLength = 0;
			ctx->nLines++;
			buf_push(ctx->tmpBuf, ch);
		}
	}
	else {
		ctx->lineLength++;
		buf_push(ctx->tmpBuf, ch);
	}

	return CONTINUE;
}

static int parse_question(struct parse_context *ctx, int inc)
{
	char ch;

	assert(ctx->state == QUESTION);

	ch = inc;
	if (inc == '\n') {
		if (ctx->lineLength == 0) {
			ctx->state++;
			init_conclusion(&ctx->conc, ctx->kb->nQuestions);
			return CONTINUE;
		} 
		else {
			ctx->kb->nQuestions++;
			buf_push(ctx->tmpBuf, '\0');
			buf_push(ctx->kb->questions, strdup(ctx->tmpBuf));
			buf_clear(ctx->tmpBuf);

			ctx->lineLength = 0;
			ctx->nQuestions++;
		}
	} 
	else {
		ctx->lineLength++;
		buf_push(ctx->tmpBuf, ch);
	}

	return CONTINUE;
}

static int parse_conc_title(struct parse_context *ctx, int inc)
{
	char ch;

	assert(ctx->state == CONCLUSION_TITLE);
	ch = inc;

	if (inc == ',') {
		buf_push(ctx->tmpBuf, '\0');
		ctx->conc.str = strdup(ctx->tmpBuf);
		ctx->fragmentSize = 0;
		ctx->state++;
		buf_clear(ctx->tmpBuf);
		return CONTINUE;
	}
	else if (inc == '\n') {
		if (ctx->lineLength > 0) {
			snprintf(ctx->kb->message, MAX_MESSAGE_LENGTH,
				"Premature end of line at row %zu column %zu",
				ctx->nLines + 1,
				ctx->lineLength);
			ctx->error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
			return STOP;
		} else {
			ctx->lineLength = 0;
		}
	}
	else {
		ctx->fragmentSize++;
		ctx->lineLength++;
		buf_push(ctx->tmpBuf, ch);
	}

}

static int parse_conc_p_apriori(struct parse_context *ctx, int inc)
{
	char ch;

	assert(ctx->state == CONCLUSION_P_APRIORI);

	ch = inc;
	if (inc == ',') {
		buf_push(ctx->tmpBuf, '\0');
		ctx->conc.probApriori = strtod(ctx->tmpBuf, NULL);
		buf_clear(ctx->tmpBuf);

		ctx->fragmentSize = 0;
		ctx->state++;
		return CONTINUE;
	}
	else if (inc == '\n') {
		snprintf(ctx->kb->message, MAX_MESSAGE_LENGTH,
			"Premature end of line at row %zu column %zu",
			ctx->nLines + 1,
			ctx->lineLength);
		ctx->error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
		return STOP;
	}
	else {
		ctx->fragmentSize++;
		ctx->lineLength++;
		buf_push(ctx->tmpBuf, ch);
	}

	return CONTINUE;
}

static int parse_conc_rule_index(struct parse_context *ctx, int inc)
{
	char ch;

	assert(ctx->state == CONCLUSION_INDEX);
	
	ch = inc;
	if (inc == ',') {
		buf_push(ctx->tmpBuf, '\0');
		ctx->iAnswerProbQuestion = atoi(ctx->tmpBuf);
		buf_clear(ctx->tmpBuf);

		ctx->fragmentSize = 0;
		ctx->state++;
		return CONTINUE;
	}
	else if (inc == '\n') {
		snprintf(ctx->kb->message, MAX_MESSAGE_LENGTH,
			"Premature end of line at row %zu column %zu",
			ctx->nLines + 1,
			ctx->lineLength);
		ctx->error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
		return STOP;
	}
	else {
		ctx->fragmentSize++;
		ctx->lineLength++;
		buf_push(ctx->tmpBuf, ch);
	}

	return CONTINUE;
}


static int parse_conc_rule_py(struct parse_context *ctx, int inc)
{
	char ch;

	assert(ctx->state == CONCLUSION_PY);

	ch = inc;
	if (inc == ',') {
		buf_push(ctx->tmpBuf, '\0');
		ctx->ansp.probYes = strtod(ctx->tmpBuf, NULL);
		buf_clear(ctx->tmpBuf);

		ctx->fragmentSize = 0;
		ctx->state++;
	}
	else if (inc == '\n') {
		snprintf(ctx->kb->message, MAX_MESSAGE_LENGTH,
			"Premature end of line at row %zu column %zu",
			ctx->nLines + 1,
			ctx->lineLength);
		ctx->error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
		return STOP;
	}
	else {
		ctx->fragmentSize++;
		ctx->lineLength++;
		buf_push(ctx->tmpBuf, ch);
	}

	return CONTINUE;
}


static int parse_conc_rule_pn(struct parse_context *ctx, int inc)
{
	char ch;

	assert(ctx->state == CONCLUSION_PN);

	ch = inc;
	if (inc == ',') {
		buf_push(ctx->tmpBuf, '\0');
		ctx->ansp.probNo = strtod(ctx->tmpBuf, NULL);
		buf_clear(ctx->tmpBuf);

		ctx->conc.answerProbs[ctx->iAnswerProbQuestion] = ctx->ansp;
		ctx->iAnswerProbQuestion = 0;

		ctx->fragmentSize = 0;
		ctx->state = CONCLUSION_INDEX;
	}
	else if (inc == '\n') {
		if (ctx->fragmentSize == 0) {
			snprintf(ctx->kb->message, MAX_MESSAGE_LENGTH,
					"Premature end of line at row %zu column %zu",
					ctx->nLines + 1,
					ctx->lineLength);
			ctx->error = KBPARSE_ERROR_PREMATURE_END_OF_LINE;
			return STOP;
		}
		else {
			buf_push(ctx->tmpBuf, '\0');
			ctx->ansp.probNo = strtod(ctx->tmpBuf, NULL);
			buf_clear(ctx->tmpBuf);

			ctx->conc.answerProbs[ctx->iAnswerProbQuestion] = ctx->ansp;
			ctx->iAnswerProbQuestion = 0;

			ctx->kb->nConclusions++;
			buf_push(ctx->kb->conclusions, ctx->conc);
			init_conclusion(&ctx->conc, ctx->kb->nQuestions);

			ctx->fragmentSize = 0;
			ctx->lineLength = 0;
			
			ctx->state = CONCLUSION_TITLE;
		}
	}
	else {
		ctx->fragmentSize++;
		ctx->lineLength++;
		buf_push(ctx->tmpBuf, ch);
	}

	return CONTINUE;
}
