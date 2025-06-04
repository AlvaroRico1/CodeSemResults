static const char *nth_line(void *data, long line)
{
	struct nth_line_cb *d = data;
	assert(d && line <= d->lines);
	assert(d->spec && d->spec->data);

	if (line == 0)
		return (char *)d->spec->data;
	else
		return (char *)d->spec->data + d->line_ends[line] + 1;
}


// Source: line-log.c
// Lines 549-559
