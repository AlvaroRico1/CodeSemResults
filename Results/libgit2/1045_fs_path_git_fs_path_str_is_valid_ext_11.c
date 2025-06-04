bool git_fs_path_str_is_valid_ext(
	const git_str *path,
	unsigned int flags,
	bool (*validate_char_cb)(char ch, void *payload),
	bool (*validate_component_cb)(const char *component, size_t len, void *payload),
	bool (*validate_length_cb)(const char *path, size_t len, size_t utf8_char_len),
	void *payload)
{
	const char *start, *c;
	size_t len = 0;

	if (!flags)
		return true;

	for (start = c = path->ptr; *c && len < path->size; c++, len++) {
		if (!validate_char(*c, flags))
			return false;

		if (validate_char_cb && !validate_char_cb(*c, payload))
			return false;

		if (*c != '/')
			continue;

		if (!validate_component(start, (c - start), flags))
			return false;

		if (validate_component_cb &&
		    !validate_component_cb(start, (c - start), payload))
			return false;

		start = c + 1;
	}

	/*
	 * We want to support paths specified as either `const char *`
	 * or `git_str *`; we pass size as `SIZE_MAX` when we use a
	 * `const char *` to avoid a `strlen`.  Ensure that we didn't
	 * have a NUL in the buffer if there was a non-SIZE_MAX length.
	 */
	if (path->size != SIZE_MAX && len != path->size)
		return false;

	if (!validate_component(start, (c - start), flags))
		return false;

	if (validate_component_cb &&
	    !validate_component_cb(start, (c - start), payload))
		return false;

#ifdef GIT_WIN32
	if ((flags & GIT_FS_PATH_REJECT_LONG_PATHS) != 0) {
		size_t utf8_len = git_utf8_char_length(path->ptr, len);

		if (!validate_length(path->ptr, len, utf8_len))
			return false;

		if (validate_length_cb &&
		    !validate_length_cb(path->ptr, len, utf8_len))
			return false;
	}
#else
	GIT_UNUSED(validate_length_cb);
#endif

	return true;
}


// Source: fs_path.c
// Lines 1659-1725
