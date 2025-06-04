static int write_each_non_note_until(const char *note_path,
		struct write_each_note_data *d)
{
	struct non_note *p = d->nn_prev;
	struct non_note *n = p ? p->next : *d->nn_list;
	int cmp = 0, ret;
	while (n && (!note_path || (cmp = strcmp(n->path, note_path)) <= 0)) {
		if (note_path && cmp == 0)
			; /* do nothing, prefer note to non-note */
		else {
			ret = write_each_note_helper(d->root, n->path, n->mode,
						     &n->oid);
			if (ret)
				return ret;
		}
		p = n;
		n = n->next;
	}
	d->nn_prev = p;
	return 0;
}


// Source: notes.c
// Lines 733-753
