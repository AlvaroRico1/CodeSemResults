fs_copy(const char *source, const char *_dest)
{
	char *dbuf = NULL;
	const char *dest = NULL;
	struct stat source_st, dest_st;

	cl_must_pass_(lstat(source, &source_st), "Failed to stat copy source");

	if (lstat(_dest, &dest_st) == 0) {
		const char *base;
		int base_len;

		/* Target exists and is directory; append basename */
		cl_assert(S_ISDIR(dest_st.st_mode));

		basename_r(&base, &base_len, source);
		cl_assert(base_len < INT_MAX);

		dbuf = joinpath(_dest, base, base_len);
		dest = dbuf;
	} else if (errno != ENOENT) {
		cl_fail("Cannot copy; cannot stat destination");
	} else {
		dest = _dest;
	}


// Source: fs.h
// Lines 436-460
