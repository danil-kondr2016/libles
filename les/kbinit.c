#include <les/knowbase.h>

#include <stdlib.h>
#include <string.h>
#include "buf.h"

LIBLES_API
KnowledgeBase *les_knowledge_base_create(void)
{
	KnowledgeBase *result;

	result = calloc(sizeof(KnowledgeBase), 1);
	return result;
}
