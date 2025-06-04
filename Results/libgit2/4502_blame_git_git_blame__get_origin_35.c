int git_blame__get_origin(
		git_blame__origin **out,
		git_blame *blame,
		git_commit *commit,
		const char *path)
{
	git_blame__entry *e;

	for (e = blame->ent; e; e = e->next) {
		if (e->suspect->commit == commit && !strcmp(e->suspect->path, path)) {
			*out = origin_incref(e->suspect);
		}
	}
	return make_origin(out, commit, path);
}


// Source: blame_git.c
// Lines 65-79
