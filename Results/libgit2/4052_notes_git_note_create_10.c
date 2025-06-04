int git_note_create(
	git_oid *out,
	git_repository *repo,
	const char *notes_ref_in,
	const git_signature *author,
	const git_signature *committer,
	const git_oid *oid,
	const char *note,
	int allow_note_overwrite)
{
	int error;
	git_str notes_ref = GIT_STR_INIT;
	git_commit *existing_notes_commit = NULL;
	git_reference *ref = NULL;
	git_oid notes_blob_oid, notes_commit_oid;

	error = retrieve_note_commit(&existing_notes_commit, &notes_ref,
			repo, notes_ref_in);

	if (error < 0 && error != GIT_ENOTFOUND)
		goto cleanup;

	error = git_note_commit_create(&notes_commit_oid,
			&notes_blob_oid,
			repo, existing_notes_commit, author,
			committer, oid, note,
			allow_note_overwrite);
	if (error < 0)
		goto cleanup;

	error = git_reference_create(&ref, repo, notes_ref.ptr,
				&notes_commit_oid, 1, NULL);

	if (out != NULL)
		git_oid_cpy(out, &notes_blob_oid);

cleanup:
	git_str_dispose(&notes_ref);
	git_commit_free(existing_notes_commit);
	git_reference_free(ref);
	return error;
}


// Source: notes.c
// Lines 528-569
