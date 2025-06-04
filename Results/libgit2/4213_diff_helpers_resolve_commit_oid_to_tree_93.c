git_tree *resolve_commit_oid_to_tree(
	git_repository *repo,
	const char *partial_oid)
{
	size_t len = strlen(partial_oid);
	git_oid oid;
	git_object *obj = NULL;
	git_tree *tree = NULL;

	if (git_oid_fromstrn(&oid, partial_oid, len) == 0)
		cl_git_pass(git_object_lookup_prefix(&obj, repo, &oid, len, GIT_OBJECT_ANY));

	cl_git_pass(git_object_peel((git_object **) &tree, obj, GIT_OBJECT_TREE));
	git_object_free(obj);
	return tree;
}


// Source: diff_helpers.c
// Lines 5-20
