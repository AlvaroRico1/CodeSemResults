static struct diff_filepair *diff_filepair_dup(struct diff_filepair *pair)
{
	struct diff_filepair *new_filepair = xmalloc(sizeof(struct diff_filepair));
	new_filepair->one = pair->one;
	new_filepair->two = pair->two;
	new_filepair->one->count++;
	new_filepair->two->count++;
	return new_filepair;
}


// Source: line-log.c
// Lines 1080-1088
