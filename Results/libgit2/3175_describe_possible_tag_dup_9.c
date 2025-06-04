static int possible_tag_dup(struct possible_tag **out, struct possible_tag *in)
{
	struct possible_tag *tag;
	int error;

	tag = git__malloc(sizeof(struct possible_tag));
	GIT_ERROR_CHECK_ALLOC(tag);

	memcpy(tag, in, sizeof(struct possible_tag));
	tag->name = NULL;

	if ((error = commit_name_dup(&tag->name, in->name)) < 0) {
		git__free(tag);
		*out = NULL;
		return error;
	}

	*out = tag;
	return 0;
}


// Source: describe.c
// Lines 256-275
