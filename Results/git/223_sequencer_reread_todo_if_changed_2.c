static int reread_todo_if_changed(struct repository *r,
				  struct todo_list *todo_list,
				  struct replay_opts *opts)
{
	int offset;
	struct strbuf buf = STRBUF_INIT;

	if (strbuf_read_file_or_whine(&buf, get_todo_path(opts)) < 0)
		return -1;
	offset = get_item_line_offset(todo_list, todo_list->current + 1);
	if (buf.len != todo_list->buf.len - offset ||
	    memcmp(buf.buf, todo_list->buf.buf + offset, buf.len)) {
		/* Reread the todo file if it has changed. */
		todo_list_release(todo_list);
		if (read_populate_todo(r, todo_list, opts))
			return -1; /* message was printed */
		/* `current` will be incremented on return */
		todo_list->current = -1;
	}


// Source: sequencer.c
// Lines 4283-4301
