static void tree_write_stack_init_subtree(struct tree_write_stack *tws,
		const char *path)
{
	struct tree_write_stack *n;
	assert(!tws->next);
	assert(tws->path[0] == '\0' && tws->path[1] == '\0');
	n = (struct tree_write_stack *)
		xmalloc(sizeof(struct tree_write_stack));
	n->next = NULL;
	strbuf_init(&n->buf, 256 * (32 + the_hash_algo->hexsz)); /* assume 256 entries per tree */
	n->path[0] = n->path[1] = '\0';
	tws->next = n;
	tws->path[0] = path[0];
	tws->path[1] = path[1];
}


// Source: notes.c
// Lines 653-667
