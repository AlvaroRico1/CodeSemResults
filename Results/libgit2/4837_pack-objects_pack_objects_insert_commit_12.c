static int pack_objects_insert_commit(git_packbuilder *pb, struct walk_object *obj)
{
	int error;
	git_commit *commit = NULL;
	git_tree *tree = NULL;

	obj->seen = 1;

	if ((error = git_packbuilder_insert(pb, &obj->id, NULL)) < 0)
		return error;

	if ((error = git_commit_lookup(&commit, pb->repo, &obj->id)) < 0)
		return error;

	if ((error = git_tree_lookup(&tree, pb->repo, git_commit_tree_id(commit))) < 0)
		goto cleanup;

	if ((error = pack_objects_insert_tree(pb, tree)) < 0)
		goto cleanup;

cleanup:
	git_commit_free(commit);
	git_tree_free(tree);
	return error;
}


// Source: pack-objects.c
// Lines 1716-1740
