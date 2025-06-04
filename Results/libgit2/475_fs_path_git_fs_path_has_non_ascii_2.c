bool git_fs_path_has_non_ascii(const char *path, size_t pathlen)
{
	const uint8_t *scan = (const uint8_t *)path, *end;

	for (end = scan + pathlen; scan < end; ++scan)
		if (*scan & 0x80)
			return true;

	return false;
}


// Source: fs_path.c
// Lines 966-975
