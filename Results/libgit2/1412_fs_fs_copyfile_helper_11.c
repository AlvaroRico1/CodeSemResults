fs_copyfile_helper(const char *source, size_t source_len, const char *dest, int dest_mode)
{
	int in, out;

	cl_must_pass((in = open(source, O_RDONLY)));
	cl_must_pass((out = open(dest, O_WRONLY|O_CREAT|O_TRUNC, dest_mode)));

#if USE_FCOPYFILE && defined(__APPLE__)
	((void)(source_len)); /* unused */
	cl_must_pass(fcopyfile(in, out, 0, COPYFILE_DATA));
#elif USE_SENDFILE && defined(__linux__)
	{
		ssize_t ret = 0;

		while (source_len && (ret = sendfile(out, in, NULL, source_len)) > 0) {
			source_len -= (size_t)ret;
		}
		cl_assert(ret >= 0);
	}


// Source: fs.h
// Lines 392-410
