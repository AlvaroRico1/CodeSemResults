static int parse_commit(git_commit **out, const char *buffer)
{
	git_commit *commit;
	git_odb_object fake_odb_object;
	int error;

	commit = (git_commit*)git__malloc(sizeof(git_commit));
	memset(commit, 0x0, sizeof(git_commit));
	commit->object.repo = g_repo;

	memset(&fake_odb_object, 0x0, sizeof(git_odb_object));
	fake_odb_object.buffer = (char *)buffer;
	fake_odb_object.cached.size = strlen(fake_odb_object.buffer);

	error = git_commit__parse(commit, &fake_odb_object);

	*out = commit;
	return error;
}


// Source: parse.c
// Lines 274-292
