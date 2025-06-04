static int compare(const void *a_, const void *b_, void *unused)
{
	const struct entry *a = a_;
	const struct entry *b = b_;
	return compare_commits_by_commit_date(a->commit, b->commit, NULL);
}


// Source: skipping.c
// Lines 53-58
