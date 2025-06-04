static int my_emit(
	long start_a, long count_a,
	long start_b, long count_b,
	void *cb_data)
{
	blame_chunk_cb_data *d = (blame_chunk_cb_data *)cb_data;

	if (blame_chunk(d->blame, d->tlno, d->plno, start_b, d->target, d->parent) < 0)
		return -1;
	d->plno = start_a + count_a;
	d->tlno = start_b + count_b;

	return 0;
}


// Source: blame_git.c
// Lines 329-342
