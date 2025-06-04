static const git_oid *commit_parent_from_commit(size_t n, void *payload)
{
	const git_commit *commit = (const git_commit *) payload;

	return git_array_get(commit->parent_ids, n);

}


// Source: commit.c
// Lines 960-966
