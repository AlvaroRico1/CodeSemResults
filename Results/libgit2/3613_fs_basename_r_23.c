static void basename_r(const char **out, int *out_len, const char *in)
{
	size_t in_len = strlen(in), start_pos;

	for (in_len = strlen(in); in_len; in_len--) {
		if (in[in_len - 1] != '/')
			break;
	}

	for (start_pos = in_len; start_pos; start_pos--) {
		if (in[start_pos - 1] == '/')
			break;
	}

	cl_assert(in_len - start_pos < INT_MAX);

	if (in_len - start_pos > 0) {
		*out = &in[start_pos];
		*out_len = (in_len - start_pos);
	} else {
		*out = "/";
		*out_len = 1;
	}
}


// Source: fs.h
// Lines 320-343
