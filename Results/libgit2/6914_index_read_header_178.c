static int read_header(struct index_header *dest, const void *buffer)
{
	const struct index_header *source = buffer;

	dest->signature = ntohl(source->signature);
	if (dest->signature != INDEX_HEADER_SIG)
		return index_error_invalid("incorrect header signature");

	dest->version = ntohl(source->version);
	if (dest->version < INDEX_VERSION_NUMBER_LB ||
		dest->version > INDEX_VERSION_NUMBER_UB)
		return index_error_invalid("incorrect header version");

	dest->entry_count = ntohl(source->entry_count);
	return 0;
}


// Source: index.c
// Lines 2561-2576
