static int write_disk_entry(git_filebuf *file, git_index_entry *entry, const char *last)
{
	void *mem = NULL;
	struct entry_short ondisk;
	size_t path_len, disk_size;
	int varint_len = 0;
	char *path;
	const char *path_start = entry->path;
	size_t same_len = 0;

	path_len = ((struct entry_internal *)entry)->pathlen;

	if (last) {
		const char *last_c = last;

		while (*path_start == *last_c) {
			if (!*path_start || !*last_c)
				break;
			++path_start;
			++last_c;
			++same_len;
		}
		path_len -= same_len;
		varint_len = git_encode_varint(NULL, 0, strlen(last) - same_len);
	}


// Source: index.c
// Lines 2753-2777
