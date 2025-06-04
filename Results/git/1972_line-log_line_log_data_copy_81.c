line_log_data_copy(struct line_log_data *r)
{
	struct line_log_data *ret = NULL;
	struct line_log_data *tmp = NULL, *prev = NULL;

	assert(r);
	ret = tmp = prev = line_log_data_copy_one(r);
	r = r->next;
	while (r) {
		tmp = line_log_data_copy_one(r);
		prev->next = tmp;
		prev = tmp;
		r = r->next;
	}

	return ret;
}


// Source: line-log.c
// Lines 639-655
