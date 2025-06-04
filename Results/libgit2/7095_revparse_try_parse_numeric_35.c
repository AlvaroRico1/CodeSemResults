static int try_parse_numeric(int *n, const char *curly_braces_content)
{
	int32_t content;
	const char *end_ptr;

	if (git__strntol32(&content, curly_braces_content, strlen(curly_braces_content),
			   &end_ptr, 10) < 0)
		return -1;

	if (*end_ptr != '\0')
		return -1;

	*n = (int)content;
	return 0;
}


// Source: revparse.c
// Lines 124-138
