cmd_wait_for_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args     	*args = cmd_get_args(self);
	const char		*name = args_string(args, 0);
	struct wait_channel	*wc, find;

	find.name = name;
	wc = RB_FIND(wait_channels, &wait_channels, &find);

	if (args_has(args, 'S'))
		return (cmd_wait_for_signal(item, name, wc));
	if (args_has(args, 'L'))
		return (cmd_wait_for_lock(item, name, wc));
	if (args_has(args, 'U'))
		return (cmd_wait_for_unlock(item, name, wc));
	return (cmd_wait_for_wait(item, name, wc));
}


// Source: cmd-wait-for.c
// Lines 121-137
