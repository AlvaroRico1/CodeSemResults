static int note_new(
	git_note **out,
	git_oid *note_oid,
	git_commit *commit,
	git_blob *blob)
{
	git_note *note = NULL;
	git_object_size_t blobsize;

	note = git__malloc(sizeof(git_note));
	GIT_ERROR_CHECK_ALLOC(note);

	git_oid_cpy(&note->id, note_oid);

	if (git_signature_dup(&note->author, git_commit_author(commit)) < 0 ||
		git_signature_dup(&note->committer, git_commit_committer(commit)) < 0)
		return -1;

	blobsize = git_blob_rawsize(blob);
	GIT_ERROR_CHECK_BLOBSIZE(blobsize);

	note->message = git__strndup(git_blob_rawcontent(blob), (size_t)blobsize);
	GIT_ERROR_CHECK_ALLOC(note->message);

	*out = note;
	return 0;
}


// Source: notes.c
// Lines 316-342
