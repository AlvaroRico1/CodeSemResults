bool git_object__is_valid(
	git_repository *repo, const git_oid *id, git_object_t expected_type)
{
	git_odb *odb;
	git_object_t actual_type;
	size_t len;
	int error;

	if (!git_object__strict_input_validation)
		return true;

	if ((error = git_repository_odb__weakptr(&odb, repo)) < 0 ||
		(error = git_odb_read_header(&len, &actual_type, odb, id)) < 0)
		return false;

	if (expected_type != GIT_OBJECT_ANY && expected_type != actual_type) {
		git_error_set(GIT_ERROR_INVALID,
			"the requested type does not match the type in the ODB");
		return false;
	}

	return true;
}


// Source: object.c
// Lines 548-570
