int git_odb_hash(git_oid *id, const void *data, size_t len, git_object_t type)
{
	git_rawobj raw;

	GIT_ASSERT_ARG(id);

	raw.data = (void *)data;
	raw.len = len;
	raw.type = type;

	return git_odb__hashobj(id, &raw);
}


// Source: odb.c
// Lines 343-354
