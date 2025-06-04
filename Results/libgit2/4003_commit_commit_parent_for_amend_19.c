static const git_oid *commit_parent_for_amend(size_t curr, void *payload)
{
	const git_commit *commit_to_amend = payload;
	if (curr >= git_array_size(commit_to_amend->parent_ids))
		return NULL;
	return git_array_get(commit_to_amend->parent_ids, curr);
}


// Source: commit.c
// Lines 320-326
