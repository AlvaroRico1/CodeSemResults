void cl_reflog_check_entry_(git_repository *repo, const char *reflog, size_t idx,
	const char *old_spec, const char *new_spec,
	const char *email, const char *message,
	const char *file, const char *func, int line)
{
	git_reflog *log;
	const git_reflog_entry *entry;
	git_str result = GIT_STR_INIT;

	cl_git_pass(git_reflog_read(&log, repo, reflog));
	entry = git_reflog_entry_byindex(log, idx);
	if (entry == NULL)
		clar__fail(file, func, line, "Reflog has no such entry", NULL, 1);

	if (old_spec) {
		git_object *obj = NULL;
		if (git_revparse_single(&obj, repo, old_spec) == GIT_OK) {
			if (git_oid_cmp(git_object_id(obj), git_reflog_entry_id_old(entry)) != 0) {
				git_oid__writebuf(&result, "\tOld OID: \"", git_object_id(obj));
				git_oid__writebuf(&result, "\" != \"", git_reflog_entry_id_old(entry));
				git_str_puts(&result, "\"\n");
			}
			git_object_free(obj);
		} else {
			git_oid *oid = git__calloc(1, sizeof(*oid));
			git_oid_fromstr(oid, old_spec);
			if (git_oid_cmp(oid, git_reflog_entry_id_old(entry)) != 0) {
				git_oid__writebuf(&result, "\tOld OID: \"", oid);
				git_oid__writebuf(&result, "\" != \"", git_reflog_entry_id_old(entry));
				git_str_puts(&result, "\"\n");
			}
			git__free(oid);
		}
	}
	if (new_spec) {
		git_object *obj = NULL;
		if (git_revparse_single(&obj, repo, new_spec) == GIT_OK) {
			if (git_oid_cmp(git_object_id(obj), git_reflog_entry_id_new(entry)) != 0) {
				git_oid__writebuf(&result, "\tNew OID: \"", git_object_id(obj));
				git_oid__writebuf(&result, "\" != \"", git_reflog_entry_id_new(entry));
				git_str_puts(&result, "\"\n");
			}
			git_object_free(obj);
		} else {
			git_oid *oid = git__calloc(1, sizeof(*oid));
			git_oid_fromstr(oid, new_spec);
			if (git_oid_cmp(oid, git_reflog_entry_id_new(entry)) != 0) {
				git_oid__writebuf(&result, "\tNew OID: \"", oid);
				git_oid__writebuf(&result, "\" != \"", git_reflog_entry_id_new(entry));
				git_str_puts(&result, "\"\n");
			}
			git__free(oid);
		}
	}


// Source: reflog_helpers.c
// Lines 31-84
