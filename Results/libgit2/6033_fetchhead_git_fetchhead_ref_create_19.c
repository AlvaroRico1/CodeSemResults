int git_fetchhead_ref_create(
	git_fetchhead_ref **out,
	git_oid *oid,
	unsigned int is_merge,
	const char *ref_name,
	const char *remote_url)
{
	git_fetchhead_ref *fetchhead_ref;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(oid);

	*out = NULL;

	fetchhead_ref = git__malloc(sizeof(git_fetchhead_ref));
	GIT_ERROR_CHECK_ALLOC(fetchhead_ref);

	memset(fetchhead_ref, 0x0, sizeof(git_fetchhead_ref));

	git_oid_cpy(&fetchhead_ref->oid, oid);
	fetchhead_ref->is_merge = is_merge;

	if (ref_name) {
		fetchhead_ref->ref_name = git__strdup(ref_name);
		GIT_ERROR_CHECK_ALLOC(fetchhead_ref->ref_name);
	}

	if (remote_url) {
		fetchhead_ref->remote_url = sanitized_remote_url(remote_url);
		GIT_ERROR_CHECK_ALLOC(fetchhead_ref->remote_url);
	}

	*out = fetchhead_ref;

	return 0;
}


// Source: fetchhead.c
// Lines 67-102
