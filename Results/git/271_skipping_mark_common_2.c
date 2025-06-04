static void mark_common(struct data *data, struct commit *c)
{
	struct commit_list *p;

	if (c->object.flags & COMMON)
		return;
	c->object.flags |= COMMON;
	if (!(c->object.flags & POPPED))
		data->non_common_revs--;

	if (!c->object.parsed)
		return;
	for (p = c->parents; p; p = p->next) {
		if (p->item->object.flags & SEEN)
			mark_common(data, p->item);
	}
}


// Source: skipping.c
// Lines 88-104
