#include <les/knowbase.h>

#include <ffsys/globals.h>
#include <ffsys/error.h>
#include <ffsys/file.h>

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
	ptrdiff_t len;
	ptrdiff_t pos;
};

static ptrdiff_t _buf_read(void *from, void *to, size_t size)
{
	struct read_buffer *rb;
	size_t i;
	ptrdiff_t ret;
	uint8_t *p;

	if (!from)
		return -1;

	rb = (struct read_buffer *)from;
	p = (uint8_t *)to;

	if (rb->pos >= rb->len)
		return 0;

	ret = rb->pos;
	for (i = 0; i < size; i++) {
		p[i] = rb->buf[rb->pos++];
		if (rb->pos >= rb->len)
			break;
	}
	ret = ret - rb->pos;

	return ret;
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

	ptrdiff_t (*read)(void *from, void *to, size_t size);
};

static ptrdiff_t _file_read(void *from, void *to, size_t size)
{
	return fffile_read((fffd)from, to, size);
}

static int parse(struct parse_context *ctx);

int les_knowledge_base_parse_file(KnowledgeBase *pKB, const char *filename)
{
	struct parse_context ctx = {0};
	fffd hInput;

	hInput = fffile_open(filename, FFFILE_READONLY);
	if (hInput == FFFILE_NULL) {
		int _error = fferr_last();
		snprintf(pKB->message, MAX_MESSAGE_LENGTH,
				"System error: %s: %s",
				filename, fferr_strptr(_error));
		return -1;
	}

	ctx.input = hInput;
	ctx.read = _file_read;
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
	ctx.read = _buf_read;
	ctx.kb = pKB;

	return parse(&ctx);
}

enum
{
	CONTINUE,
	STOP,
};

static int parse_comment(struct parse_context *ctx, uint8_t inc);
static int parse_question(struct parse_context *ctx, uint8_t inc);
static int parse_conc_title(struct parse_context *ctx, uint8_t inc);
static int parse_conc_p_apriori(struct parse_context *ctx, uint8_t inc);
static int parse_conc_rule_index(struct parse_context *ctx, uint8_t inc);
static int parse_conc_rule_py(struct parse_context *ctx, uint8_t inc);
static int parse_conc_rule_pn(struct parse_context *ctx, uint8_t inc);

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

#define BUF_SIZE 4096
static int parse(struct parse_context *ctx)
{
	uint8_t buf[BUF_SIZE];
	ptrdiff_t n_read = 0, pos = 0;
	int eof = 0;
	uint8_t inc, postcr;
	int ret, cr = 0;

	ctx->nLines = 0;
	ctx->lineLength = 0;
	ctx->state = 0;
	ctx->nQuestions = 0;
	ctx->fragmentSize = 0;

	do {
		if (pos >= n_read) {
			pos = 0;
			n_read = ctx->read(ctx->input, buf, BUF_SIZE);
		}
		
		/*
		if (n_read == 0) {
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
		 */

		if (cr == 1) {
			inc = '\r';
			cr = 2;
		} else if (cr == 2) {
			inc = postcr;
			cr = 0;
		} else {
			inc = buf[pos];
			if (inc == '\r') {
				cr = 1;
				pos++;
				if (pos >= n_read) {
					pos = 0;
					n_read = ctx->read(ctx->input, buf, BUF_SIZE);
				}
				inc = buf[pos];
				pos++;
				if (inc == '\n')
					cr = 0;
				else 
					postcr = inc;
			} else {
				pos++;
			}
		}

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
	} while (n_read > 0);

	buf_free(ctx->tmpBuf);

	return ctx->error;
}

static int parse_comment(struct parse_context *ctx, uint8_t inc)
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

static int parse_question(struct parse_context *ctx, uint8_t inc)
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

static int parse_conc_title(struct parse_context *ctx, uint8_t inc)
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

static int parse_conc_p_apriori(struct parse_context *ctx, uint8_t inc)
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

static int parse_conc_rule_index(struct parse_context *ctx, uint8_t inc)
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


static int parse_conc_rule_py(struct parse_context *ctx, uint8_t inc)
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


static int parse_conc_rule_pn(struct parse_context *ctx, uint8_t inc)
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
