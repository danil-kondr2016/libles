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

static int parse(KnowledgeBaseParser *ctx);

LIBLES_API
void les_knowledge_base_init_parser(KnowledgeBaseParser *pParser)
{
	assert(pParser);

	pParser->tmpBuf = NULL;
	pParser->kb = NULL;
	pParser->conc.str = NULL;
	pParser->conc.probApriori = 0;
	pParser->conc.answerProbs = NULL;
	pParser->ansp.probYes = 0;
	pParser->ansp.probNo = 0;
	pParser->nLines = 0;
	pParser->lineLength = 0;
	pParser->state = 0;
	pParser->nQuestions = 0;
	pParser->fragmentSize = 0;
	pParser->error = 0;
	pParser->iAnswerProbQuestion = 0;
}

LIBLES_API
int les_knowledge_base_parse(KnowledgeBaseParser *pParser, KnowledgeBase *pKB)
{
	assert(pParser);
	assert(pKB);

	pParser->kb = pKB;
	return parse(pParser);
}

enum
{
	CONTINUE,
	STOP,
};

static int parse_comment(KnowledgeBaseParser *ctx, int inc);
static int parse_question(KnowledgeBaseParser *ctx, int inc);
static int parse_conc_title(KnowledgeBaseParser *ctx, int inc);
static int parse_conc_p_apriori(KnowledgeBaseParser *ctx, int inc);
static int parse_conc_rule_index(KnowledgeBaseParser *ctx, int inc);
static int parse_conc_rule_py(KnowledgeBaseParser *ctx, int inc);
static int parse_conc_rule_pn(KnowledgeBaseParser *ctx, int inc);

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
static int parse(KnowledgeBaseParser *ctx)
{
	uint8_t buf[BUF_SIZE];
	ptrdiff_t n_read = 0, pos = 0;
	int eof = 0;
	int inc, postcr;
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

		
		if (n_read == 0) {
			inc = -1;
		}
		else if (cr == 1) {
			inc = '\r';
			cr = 2;
		} 
		else if (cr == 2) {
			inc = postcr;
			cr = 0;
		} 
		else {
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

static int parse_comment(KnowledgeBaseParser *ctx, int inc)
{
	char ch;

	assert(ctx->state == COMMENT);

	ch = inc;
	if (inc == '\n' || inc == -1) {
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

static int parse_question(KnowledgeBaseParser *ctx, int inc)
{
	char ch;

	assert(ctx->state == QUESTION);

	ch = inc;
	if (inc == '\n' || inc == -1) {
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

static int parse_conc_title(KnowledgeBaseParser *ctx, int inc)
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
	else if (inc == '\n' || inc == -1) {
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

static int parse_conc_p_apriori(KnowledgeBaseParser *ctx, int inc)
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
	else if (inc == '\n' || inc == -1) {
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

static int parse_conc_rule_index(KnowledgeBaseParser *ctx, int inc)
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
	else if (inc == '\n' || inc == -1) {
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


static int parse_conc_rule_py(KnowledgeBaseParser *ctx, int inc)
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
	else if (inc == '\n' || inc == -1) {
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


static int parse_conc_rule_pn(KnowledgeBaseParser *ctx, int inc)
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
	else if (inc == '\n' || inc == -1) {
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
