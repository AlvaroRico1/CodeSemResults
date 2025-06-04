static const git_oid *commit_parent_from_varargs(size_t curr, void *payload)
{
	commit_parent_varargs *data = payload;
	const git_commit *commit;
	if (curr >= data->total)
		return NULL;
	commit = va_arg(data->args, const git_commit *);
	return commit ? git_commit_id(commit) : NULL;
}


// Source: commit.c
// Lines 208-216
