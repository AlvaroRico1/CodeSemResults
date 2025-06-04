static int merge_conflict_invoke_driver(
	git_index_entry **out,
	const char *name,
	git_merge_driver *driver,
	git_merge_diff_list *diff_list,
	git_merge_driver_source *src)
{
	git_index_entry *result;
	git_buf buf = {0};
	const char *path;
	uint32_t mode;
	git_odb *odb = NULL;
	git_oid oid;
	int error;

	*out = NULL;

	if ((error = driver->apply(driver, &path, &mode, &buf, name, src)) < 0 ||
		(error = git_repository_odb(&odb, src->repo)) < 0 ||
		(error = git_odb_write(&oid, odb, buf.ptr, buf.size, GIT_OBJECT_BLOB)) < 0)
		goto done;

	result = git_pool_mallocz(&diff_list->pool, sizeof(git_index_entry));
	GIT_ERROR_CHECK_ALLOC(result);

	git_oid_cpy(&result->id, &oid);
	result->mode = mode;
	result->file_size = (uint32_t)buf.size;

	result->path = git_pool_strdup(&diff_list->pool, path);
	GIT_ERROR_CHECK_ALLOC(result->path);

	*out = result;

done:
	git_buf_dispose(&buf);
	git_odb_free(odb);

	return error;
}


// Source: merge.c
// Lines 888-927
