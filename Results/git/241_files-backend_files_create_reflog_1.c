static int files_create_reflog(struct ref_store *ref_store,
			       const char *refname, int force_create,
			       struct strbuf *err)
{
	struct files_ref_store *refs =
		files_downcast(ref_store, REF_STORE_WRITE, "create_reflog");
	int fd;

	if (log_ref_setup(refs, refname, force_create, &fd, err))
		return -1;

	if (fd >= 0)
		close(fd);

	return 0;
}


// Source: files-backend.c
// Lines 1636-1651
