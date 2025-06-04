static int maybe_append_head(refdb_fs_backend *backend, const git_reference *ref, const git_signature *who, const char *message)
{
	git_reference *head = NULL;
	git_refdb *refdb = NULL;
	int error, write_reflog;
	git_oid old_id;

	if ((error = git_repository_refdb(&refdb, backend->repo)) < 0 ||
	    (error = git_refdb_should_write_head_reflog(&write_reflog, refdb, ref)) < 0)
		goto out;
	if (!write_reflog)
		goto out;

	/* if we can't resolve, we use {0}*40 as old id */
	if (git_reference_name_to_id(&old_id, backend->repo, ref->name) < 0)
		memset(&old_id, 0, sizeof(old_id));

	if ((error = git_reference_lookup(&head, backend->repo, GIT_HEAD_FILE)) < 0 ||
	    (error = reflog_append(backend, head, &old_id, git_reference_target(ref), who, message)) < 0)
		goto out;

out:
	git_reference_free(head);
	git_refdb_free(refdb);
	return error;
}


// Source: refdb_fs.c
// Lines 1484-1509
