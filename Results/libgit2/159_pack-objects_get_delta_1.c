static int get_delta(void **out, git_odb *odb, git_pobject *po)
{
	git_odb_object *src = NULL, *trg = NULL;
	size_t delta_size;
	void *delta_buf;
	int error;

	*out = NULL;

	if (git_odb_read(&src, odb, &po->delta->id) < 0 ||
	    git_odb_read(&trg, odb, &po->id) < 0)
		goto on_error;

	error = git_delta(&delta_buf, &delta_size,
		git_odb_object_data(src), git_odb_object_size(src),
		git_odb_object_data(trg), git_odb_object_size(trg),
		0);

	if (error < 0 && error != GIT_EBUFS)
		goto on_error;

	if (error == GIT_EBUFS || delta_size != po->delta_size) {
		git_error_set(GIT_ERROR_INVALID, "delta size changed");
		goto on_error;
	}

	*out = delta_buf;

	git_odb_object_free(src);
	git_odb_object_free(trg);
	return 0;

on_error:
	git_odb_object_free(src);
	git_odb_object_free(trg);
	return -1;
}


// Source: pack-objects.c
// Lines 270-306
