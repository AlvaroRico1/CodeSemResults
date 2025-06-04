static void overwrite_filemode(const char *expected, git_buf *actual)
{
	size_t offset;
	char *found;

	found = strstr(expected, "100644");
	if (!found)
		return;

	offset = ((const char *)found) - expected;
	if (actual->size < offset + 6)
		return;

	if (memcmp(&actual->ptr[offset], "100644", 6) != 0)
		memcpy(&actual->ptr[offset], "100644", 6);
}


// Source: drivers.c
// Lines 18-33
