#include <les/knowbase.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <errno.h>

#include "buf.h"
#include "kbparser.h"

static int __cp1251_to_unicode[256] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
	0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021,
	0x20ac, 0x2030, 0x0409, 0x2039, 0x040a, 0x040c, 0x040b, 0x040f,
	0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
	0xfffd, 0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f,
	0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7,
	0x0401, 0x00a9, 0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407,
	0x00b0, 0x00b1, 0x0406, 0x0456, 0x0491, 0x00b5, 0x00b6, 0x00b7,
	0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405, 0x0455, 0x0457,
	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
	0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
	0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
	0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
	0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
	0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
	0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
	0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
};

static int cp1251_to_unicode(uint8_t x)
{
	return __cp1251_to_unicode[(int)x];
}

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
	KBPARSE_ERROR_NON_EXISTENT_QUESTION,
};

static int parse(KBParser *ctx);

LIBLES_API
KBParser *les_knowledge_base_create_parser(
		uintptr_t input, KBParserReadFn read)
{
	KBParser *result;
       
	result = calloc(sizeof(KBParser), 1);
	if (!result)
		return NULL;

	result->input = input;
	result->read = read;

	return result;
}

LIBLES_API
void les_knowledge_base_destroy_parser(KBParser **pParser)
{
	KBParser *parser;

	if (!pParser)
		return;

	if (!*pParser)
		return;

	parser = *pParser;

	buf_free(parser->tmpBuf);
	buf_free(parser->conc.str);
	buf_free(parser->conc.answerProbs);
	free(parser);
	*pParser = NULL;
}

LIBLES_API
int les_knowledge_base_parse(KBParser *pParser, KnowledgeBase *pKB)
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

static int parse_comment(KBParser *ctx, int inc);
static int parse_question(KBParser *ctx, int inc);
static int parse_conc_title(KBParser *ctx, int inc);
static int parse_conc_p_apriori(KBParser *ctx, int inc);
static int parse_conc_rule_index(KBParser *ctx, int inc);
static int parse_conc_rule_py(KBParser *ctx, int inc);
static int parse_conc_rule_pn(KBParser *ctx, int inc);

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
static int parse(KBParser *ctx)
{
	uint8_t buf[BUF_SIZE] = {0};
	ptrdiff_t n_read = 0, pos = 0;
	int eof = 0;
	int inc, postcr;
	int ret, cr = 0, utf8 = 0, uch = 0;

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

		if (ctx->state == COMMENT) {
			if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) {
				pos = 3;
				ctx->encoding = LES_KB_UTF8;
			}
			else {
				ctx->encoding = LES_KB_CP1251;
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

			if (inc >= 0x80 && inc <= 0x7ff) {
				utf8 = 2;
			}
			else if (inc >= 0x800 && inc <= 0xffff) {
				utf8 = 5;
			}

		} 
		else if (ctx->encoding == LES_KB_UTF8) {
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
		else if (utf8 == 0 && ctx->encoding == LES_KB_CP1251) {
			inc = cp1251_to_unicode(buf[pos]);
			if (inc >= 0x80 && inc <= 0x7ff) {
				utf8 = 2;
			}
			else if (inc >= 0x800 && inc <= 0xffff) {
				utf8 = 5;
			}

			if (inc == '\r') {
				cr = 1;
				pos++;
				if (pos >= n_read) {
					pos = 0;
					n_read = ctx->read(ctx->input, buf, BUF_SIZE);
				}

				inc = cp1251_to_unicode(buf[pos]);
				pos++;
				if (inc == '\n')
					cr = 0;
				else 
					postcr = inc;
			} else {
				pos++;
			}

		}

		if (utf8 && !uch) {
			uch = inc;
		}

		if (utf8 == 2) {
			utf8 = 1;
			inc = 0xC0 | ((uch >> 6) & 0x1f);
		}
		else if (utf8 == 1) {
			utf8 = 0;
			inc = 0x80 | (uch & 0x3f);
			uch = 0;
		}
		else if (utf8 == 5) {
			utf8 = 4;
			inc = 0xE0 | ((uch >> 12) & 0x0f);
		}
		else if (utf8 == 4) {
			utf8 = 1;
			inc = 0x80 | ((uch >> 6) & 0x3f);
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

static int parse_comment(KBParser *ctx, int inc)
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

static int parse_question(KBParser *ctx, int inc)
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

static int parse_conc_title(KBParser *ctx, int inc)
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

static int parse_conc_p_apriori(KBParser *ctx, int inc)
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

static int parse_conc_rule_index(KBParser *ctx, int inc)
{
	char ch;

	assert(ctx->state == CONCLUSION_INDEX);
	
	ch = inc;
	if (inc == ',') {
		buf_push(ctx->tmpBuf, '\0');
		ctx->iAnswerProbQuestion = atoi(ctx->tmpBuf);
		if (ctx->iAnswerProbQuestion >= ctx->kb->nQuestions) {
			snprintf(ctx->kb->message, MAX_MESSAGE_LENGTH,
				"Reference to non-existent question #%d "
				"at row %zu column %zu",
				ctx->iAnswerProbQuestion,
				ctx->nLines + 1,
				ctx->lineLength);
			ctx->error = KBPARSE_ERROR_NON_EXISTENT_QUESTION;
			return STOP;
		}
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


static int parse_conc_rule_py(KBParser *ctx, int inc)
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


static int parse_conc_rule_pn(KBParser *ctx, int inc)
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
