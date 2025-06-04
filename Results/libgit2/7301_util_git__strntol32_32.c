int git__strntol32(int32_t *result, const char *nptr, size_t nptr_len, const char **endptr, int base)
{
	const char *tmp_endptr;
	int32_t tmp_int;
	int64_t tmp_long;
	int error;

	if ((error = git__strntol64(&tmp_long, nptr, nptr_len, &tmp_endptr, base)) < 0)
		return error;

	tmp_int = tmp_long & 0xFFFFFFFF;
	if (tmp_int != tmp_long) {
		int len = (int)(tmp_endptr - nptr);
		git_error_set(GIT_ERROR_INVALID, "failed to convert: '%.*s' is too large", len, nptr);
		return -1;
	}

	*result = tmp_int;
	if (endptr)
		*endptr = tmp_endptr;

	return error;
}


// Source: util.c
// Lines 138-160
