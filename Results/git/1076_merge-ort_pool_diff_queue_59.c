static struct diff_filepair *pool_diff_queue(struct mem_pool *pool,
					     struct diff_queue_struct *queue,
					     struct diff_filespec *one,
					     struct diff_filespec *two)
{
	/* Same code as diff_queue(), except allocate from pool */
	struct diff_filepair *dp;

	dp = mem_pool_calloc(pool, 1, sizeof(*dp));
	dp->one = one;
	dp->two = two;
	if (queue)
		diff_q(queue, dp);
	return dp;
}


// Source: merge-ort.c
// Lines 664-678
