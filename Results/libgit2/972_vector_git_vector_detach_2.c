void **git_vector_detach(size_t *size, size_t *asize, git_vector *v)
{
	void **data = v->contents;

	if (size)
		*size = v->length;
	if (asize)
		*asize = v->_alloc_size;

	v->_alloc_size = 0;
	v->length   = 0;
	v->contents = NULL;

	return data;
}


// Source: vector.c
// Lines 119-133
