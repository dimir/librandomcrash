#include <stdlib.h>
#include "handlers.h"

struct handler *acct_handlers[] = {
	NULL,
};

struct handler *handlers[] = {
	&read_randomcase,
	NULL,
};
