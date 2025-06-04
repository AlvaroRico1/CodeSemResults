int git_note_commit_remove(
		git_oid *notes_commit_out,
		git_repository *repo,
		git_commit *notes_commit,
		const git_signature *author,
		const git_signature *committer,
		const git_oid *oid)
{
	int error;
	git_tree *tree = NULL;
	char target[GIT_OID_HEXSZ + 1];

	git_oid_tostr(target, sizeof(target), oid);

	if ((error = git_commit_tree(&tree, notes_commit)) < 0)
		goto cleanup;

	error = note_remove(notes_commit_out,
		repo, author, committer, NULL, tree, target, &notes_commit);

cleanup:
	git_tree_free(tree);
	return error;
}


// Source: notes.c
// Lines 571-594
