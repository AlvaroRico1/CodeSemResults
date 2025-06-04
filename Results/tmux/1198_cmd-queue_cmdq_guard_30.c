cmdq_guard(struct cmdq_item *item, const char *guard, int flags)
{
	struct client	*c = item->client;
	long		 t = item->time;
	u_int		 number = item->number;

	if (c != NULL && (c->flags & CLIENT_CONTROL))
		control_write(c, "%%%s %ld %u %d", guard, t, number, flags);
}


// Source: cmd-queue.c
// Lines 798-806
