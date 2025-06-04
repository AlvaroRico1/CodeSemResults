int git_note_commit_create(
	git_oid *notes_commit_out,
	git_oid *notes_blob_out,
	git_repository *repo,
	git_commit *parent,
	const git_signature *author,
	const git_signature *committer,
	const git_oid *oid,
	const char *note,
	int allow_note_overwrite)
{
	int error;
	git_tree *tree = NULL;
	char target[GIT_OID_HEXSZ + 1];

	git_oid_tostr(target, sizeof(target), oid);

	if (parent != NULL && (error = git_commit_tree(&tree, parent)) < 0)
		goto cleanup;

	error = note_write(notes_commit_out, notes_blob_out, repo, author,
			committer, NULL, note, tree, target, &parent, allow_note_overwrite);

	if (error < 0)
		goto cleanup;

cleanup:
	git_tree_free(tree);
	return error;
}


// Source: notes.c
// Lines 497-526
