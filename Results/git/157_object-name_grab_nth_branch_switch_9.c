static int grab_nth_branch_switch(struct object_id *ooid, struct object_id *noid,
				  const char *email, timestamp_t timestamp, int tz,
				  const char *message, void *cb_data)
{
	struct grab_nth_branch_switch_cbdata *cb = cb_data;
	const char *match = NULL, *target = NULL;
	size_t len;

	if (skip_prefix(message, "checkout: moving from ", &match))
		target = strstr(match, " to ");

	if (!match || !target)
		return 0;
	if (--(cb->remaining) == 0) {
		len = target - match;
		strbuf_reset(cb->sb);
		strbuf_add(cb->sb, match, len);
		return 1; /* we are done */
	}
	return 0;
}


// Source: object-name.c
// Lines 1289-1309
