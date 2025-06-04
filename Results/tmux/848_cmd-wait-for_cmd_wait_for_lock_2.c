cmd_wait_for_lock(struct cmdq_item *item, const char *name,
    struct wait_channel *wc)
{
	struct wait_item	*wi;

	if (cmdq_get_client(item) == NULL) {
		cmdq_error(item, "not able to lock");
		return (CMD_RETURN_ERROR);
	}

	if (wc == NULL)
		wc = cmd_wait_for_add(name);

	if (wc->locked) {
		wi = xcalloc(1, sizeof *wi);
		wi->item = item;
		TAILQ_INSERT_TAIL(&wc->lockers, wi, entry);
		return (CMD_RETURN_WAIT);
	}
	wc->locked = 1;

	return (CMD_RETURN_NORMAL);
}


// Source: cmd-wait-for.c
// Lines 196-218
