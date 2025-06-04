GIT_INLINE(int) resize_vector(git_vector *v, size_t new_size)
{
	void *new_contents;

	if (new_size == 0)
		return 0;

	new_contents = git__reallocarray(v->contents, new_size, sizeof(void *));
	GIT_ERROR_CHECK_ALLOC(new_contents);

	v->_alloc_size = new_size;
	v->contents = new_contents;

	return 0;
}


// Source: vector.c
// Lines 31-45
