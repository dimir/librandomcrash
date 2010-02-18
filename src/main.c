#include "override.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>

#include "log.h"

static const char my_name[] = "librandomcrash";
static const char my_ver[] = "0.0.1";

void basic_nastiness(struct override *o)
{
	log_print(LL_OINFO, "--------------------\n");
}

extern struct override *lrc_overrides[];
extern struct handler *acct_handlers[];
extern struct handler *handlers[];

#define MAXQUEUE 32

int __lrc_call_entry(struct override *o, void *ctxp)
{
	struct handler *queue[MAXQUEUE];
	int i, qlast = 0;

	log_print(LL_PINFO, "%s() entry\n", o->name);

	/* first, run the first enabled accounting handler for this call */
	for (i = 0; acct_handlers[i]; i++)
		if (!strcmp(acct_handlers[i]->fn_name, o->name) &&
		    acct_handlers[i]->enabled &&
		    acct_handlers[i]->probe_func &&
		    !acct_handlers[i]->probe_func(o, ctxp))
			break;

	for (i = 0; handlers[i] && qlast < MAXQUEUE; i++)
		if (!strcmp(handlers[i]->fn_name, o->name) &&
		    handlers[i]->enabled &&
		    handlers[i]->probe_func &&
		    !handlers[i]->probe_func(o, ctxp))
			queue[qlast++] = handlers[i];

	if (!qlast) {
		log_print(LL_OWARN, "no handlers for %s call\n", o->name);
		return 0;
	}

	/* XXX: need to pick a random one */
	queue[0]->entry_func(o, ctxp);

	return 0;
}

void __lrc_call_exit(struct override *o, void *ctxp, void *retp)
{
	int i;
	struct lrcpriv_callctx *callctx =
		&((struct __lrc_callctx *)ctxp)->callctx;

	log_print(LL_PINFO, "%s() exit, ret=%d\n", o->name,
		  retp ? *(int *)retp : 0);

	if (callctx->acct_handler && callctx->acct_handler->exit_func)
		callctx->acct_handler->exit_func(o, ctxp, retp);

	if (callctx->handler && callctx->handler->exit_func)
		callctx->handler->exit_func(o, ctxp, retp);
}

void __ctor lrc_init(void)
{
	int i;

	log_init();
	log_print(LL_OINFO, "%s, %s initializing\n", my_name, my_ver);
	for (i = 0; lrc_overrides[i]; i++) {
		log_print(LL_OINFO, "=> loading %s wrapper\n",
			lrc_overrides[i]->name);
		lrc_overrides[i]->orig_func = dlsym(RTLD_NEXT,
						    lrc_overrides[i]->name);
	}
}

void __dtor lrc_done(void)
{
}

#ifdef __YEAH_RIGHT__
int main (void)
{
        return 0;
}
#endif
