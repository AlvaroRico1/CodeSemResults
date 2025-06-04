static int commit_name_dup(struct commit_name **out, struct commit_name *in)
{
	struct commit_name *name;

	name = git__malloc(sizeof(struct commit_name));
	GIT_ERROR_CHECK_ALLOC(name);

	memcpy(name, in,  sizeof(struct commit_name));
	name->tag = NULL;
	name->path = NULL;

	if (in->tag && git_tag_dup(&name->tag, in->tag) < 0)
		return -1;

	name->path = git__strdup(in->path);
	GIT_ERROR_CHECK_ALLOC(name->path);

	*out = name;
	return 0;
}


// Source: describe.c
// Lines 181-200
