static int packed_commit__cmp(const void *a_, const void *b_)
{
	const struct packed_commit *a = a_;
	const struct packed_commit *b = b_;
	return git_oid_cmp(&a->sha1, &b->sha1);
}


// Source: commit_graph.c
// Lines 619-624
