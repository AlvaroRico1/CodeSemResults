control_error(struct cmdq_item *item, void *data)
{
	struct client	*c = cmdq_get_client(item);
	char		*error = data;

	cmdq_guard(item, "begin", 1);
	control_write(c, "parse error: %s", error);
	cmdq_guard(item, "error", 1);

	free(error);
	return (CMD_RETURN_NORMAL);
}


// Source: control.c
// Lines 522-533
