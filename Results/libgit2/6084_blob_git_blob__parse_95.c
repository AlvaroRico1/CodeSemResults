int git_blob__parse(void *_blob, git_odb_object *odb_obj)
{
	git_blob *blob = (git_blob *) _blob;

	GIT_ASSERT_ARG(blob);

	git_cached_obj_incref((git_cached_obj *)odb_obj);
	blob->raw = 0;
	blob->data.odb = odb_obj;
	return 0;
}


// Source: blob.c
// Lines 67-77
