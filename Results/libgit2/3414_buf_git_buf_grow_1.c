int git_buf_grow(git_buf *buffer, size_t target_size)
{
	char *newptr;

	if (buffer->reserved >= target_size)
		return 0;

	if (buffer->ptr == git_str__initstr)
		newptr = git__malloc(target_size);
	else
		newptr = git__realloc(buffer->ptr, target_size);

	if (!newptr)
		return -1;

	buffer->ptr = newptr;
	buffer->reserved = target_size;
	return 0;
}


// Source: buf.c
// Lines 73-91
