static int add_to_known_names(
	git_repository *repo,
	git_oidmap *names,
	const char *path,
	const git_oid *peeled,
	unsigned int prio,
	const git_oid *sha1)
{
	struct commit_name *e = find_commit_name(names, peeled);
	bool found = (e != NULL);

	git_tag *tag = NULL;
	if (replace_name(&tag, repo, e, prio, sha1)) {
		if (!found) {
			e = git__malloc(sizeof(struct commit_name));
			GIT_ERROR_CHECK_ALLOC(e);

			e->path = NULL;
			e->tag = NULL;
		}

		if (e->tag)
			git_tag_free(e->tag);
		e->tag = tag;
		e->prio = prio;
		e->name_checked = 0;
		git_oid_cpy(&e->sha1, sha1);
		git__free(e->path);
		e->path = git__strdup(path);
		git_oid_cpy(&e->peeled, peeled);

		if (!found && git_oidmap_set(names, &e->peeled, e) < 0)
			return -1;
	}
	else
		git_tag_free(tag);

	return 0;
}


// Source: describe.c
// Lines 93-131
