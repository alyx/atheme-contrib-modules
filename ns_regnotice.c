/*
 * Copyright (c) 2010 William Pitcock <nenolod@atheme.org>
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Sends a customized welcome message on nickname registration.
 */

#include "atheme-compat.h"

#if (CURRENT_ABI_REVISION < 730000)
#  include "conf.h"
#endif

static mowgli_list_t regnotices = { NULL, NULL, 0 };

static void
regnotice_hook(myuser_t *mu)
{
	mowgli_node_t *n;

	return_if_fail(mu != NULL);

	MOWGLI_ITER_FOREACH(n, regnotices.head)
	{
		char *line = n->data;

		myuser_notice(nicksvs.nick, mu, "%s", line);
	}
}

static int
regnotice_config_handler(mowgli_config_file_entry_t *ce)
{
	mowgli_config_file_entry_t *cce;

	MOWGLI_ITER_FOREACH(cce, ce->entries)
	{
		char *line = sstrdup(cce->varname);
		mowgli_node_add(line, mowgli_node_create(), &regnotices);
	}

	return 0;
}

static void
regnotice_config_purge(void *unused)
{
	mowgli_node_t *n, *tn;

	MOWGLI_ITER_FOREACH_SAFE(n, tn, regnotices.head)
	{
		char *line = n->data;

		sfree(line);
		mowgli_node_delete(n, &regnotices);
		mowgli_node_free(n);
	}
}

static void
mod_init(module_t *m)
{
	hook_add_event("config_purge");
	hook_add_config_purge(regnotice_config_purge);

	hook_add_event("user_register");
	hook_add_user_register(regnotice_hook);

	add_conf_item("REGNOTICE", &nicksvs.me->conf_table, regnotice_config_handler);
}

static void
mod_deinit(module_unload_intent_t intent)
{
	hook_del_user_register(regnotice_hook);
	hook_del_config_purge(regnotice_config_purge);

	del_conf_item("REGNOTICE", &nicksvs.me->conf_table);
}

SIMPLE_DECLARE_MODULE_V1("contrib/ns_regnotice", MODULE_UNLOAD_CAPABILITY_OK)
