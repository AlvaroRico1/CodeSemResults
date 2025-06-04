static struct line_log_data *line_log_data_merge(struct line_log_data *a,
						 struct line_log_data *b)
{
	struct line_log_data *head = NULL, **pp = &head;

	while (a || b) {
		struct line_log_data *src;
		struct line_log_data *src2 = NULL;
		struct line_log_data *d;
		int cmp;
		if (!a)
			cmp = 1;
		else if (!b)
			cmp = -1;
		else
			cmp = strcmp(a->path, b->path);
		if (cmp < 0) {
			src = a;
			a = a->next;
		} else if (cmp == 0) {
			src = a;
			a = a->next;
			src2 = b;
			b = b->next;
		} else {
			src = b;
			b = b->next;
		}
		d = xmalloc(sizeof(struct line_log_data));
		line_log_data_init(d);
		d->path = xstrdup(src->path);
		*pp = d;
		pp = &d->next;
		if (src2)
			range_set_union(&d->ranges, &src->ranges, &src2->ranges);
		else
			range_set_copy(&d->ranges, &src->ranges);
	}

	return head;
}


// Source: line-log.c
// Lines 658-698
