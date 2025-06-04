static int object_entry__cmp(const void *a_, const void *b_)
{
	const git_midx_entry *a = (const git_midx_entry *)a_;
	const git_midx_entry *b = (const git_midx_entry *)b_;

	return git_oid_cmp(&a->sha1, &b->sha1);
}


// Source: midx.c
// Lines 585-591
