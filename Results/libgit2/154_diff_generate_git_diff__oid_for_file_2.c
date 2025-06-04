int git_diff__oid_for_file(
	git_oid *out,
	git_diff *diff,
	const char *path,
	uint16_t mode,
	git_object_size_t size)
{
	git_index_entry entry;

	if (size > UINT32_MAX) {
		git_error_set(GIT_ERROR_NOMEMORY, "file size overflow (for 32-bits) on '%s'", path);
		return -1;
	}

	memset(&entry, 0, sizeof(entry));
	entry.mode = mode;
	entry.file_size = (uint32_t)size;
	entry.path = (char *)path;

	return git_diff__oid_for_entry(out, diff, &entry, mode, NULL);
}


// Source: diff_generate.c
// Lines 583-603
