static void fix_filename(const char *prefix, const char **file)
{
	if (!file || !*file || !prefix || is_absolute_path(*file)
	    || !strcmp("-", *file))
		return;
	*file = prefix_filename(prefix, *file);
}


// Source: parse-options.c
// Lines 42-48
