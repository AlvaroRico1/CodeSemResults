paste_add(const char *prefix, char *data, size_t size)
{
	struct paste_buffer	*pb, *pb1;
	u_int			 limit;

	if (prefix == NULL)
		prefix = "buffer";

	if (size == 0) {
		free(data);
		return;
	}

	limit = options_get_number(global_options, "buffer-limit");
	RB_FOREACH_REVERSE_SAFE(pb, paste_time_tree, &paste_by_time, pb1) {
		if (paste_num_automatic < limit)
			break;
		if (pb->automatic)
			paste_free(pb);
	}

	pb = xmalloc(sizeof *pb);

	pb->name = NULL;
	do {
		free(pb->name);
		xasprintf(&pb->name, "%s%u", prefix, paste_next_index);
		paste_next_index++;
	} while (paste_get_name(pb->name) != NULL);

	pb->data = data;
	pb->size = size;

	pb->automatic = 1;
	paste_num_automatic++;

	pb->created = time(NULL);

	pb->order = paste_next_order++;
	RB_INSERT(paste_name_tree, &paste_by_name, pb);
	RB_INSERT(paste_time_tree, &paste_by_time, pb);
}


// Source: paste.c
// Lines 160-201
