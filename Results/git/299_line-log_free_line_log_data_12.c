static void free_line_log_data(struct line_log_data *r)
{
	while (r) {
		struct line_log_data *next = r->next;
		line_log_data_clear(r);
		free(r);
		r = next;
	}


// Source: line-log.c
// Lines 254-261
