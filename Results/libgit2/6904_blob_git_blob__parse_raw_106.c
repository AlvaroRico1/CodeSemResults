int git_blob__parse_raw(void *_blob, const char *data, size_t size)
{
	git_blob *blob = (git_blob *) _blob;

	GIT_ASSERT_ARG(blob);

	blob->raw = 1;
	blob->data.raw.data = data;
	blob->data.raw.size = size;
	return 0;
}


// Source: blob.c
// Lines 55-65
