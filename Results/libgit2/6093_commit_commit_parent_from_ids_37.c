static const git_oid *commit_parent_from_ids(size_t curr, void *payload)
{
	commit_parent_oids *data = payload;
	return (curr < data->total) ? data->parents[curr] : NULL;
}


// Source: commit.c
// Lines 253-257
