static const git_oid *commit_parent_from_array(size_t curr, void *payload)
{
	commit_parent_data *data = payload;
	const git_commit *commit;
	if (curr >= data->total)
		return NULL;
	commit = data->parents[curr];
	if (git_commit_owner(commit) != data->repo)
		return NULL;
	return git_commit_id(commit);
}


// Source: commit.c
// Lines 285-295
