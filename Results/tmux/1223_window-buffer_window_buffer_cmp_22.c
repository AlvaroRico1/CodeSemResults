window_buffer_cmp(const void *a0, const void *b0)
{
	const struct window_buffer_itemdata *const	*a = a0;
	const struct window_buffer_itemdata *const	*b = b0;
	int						 result = 0;

	if (window_buffer_sort->field == WINDOW_BUFFER_BY_TIME)
		result = (*b)->order - (*a)->order;
	else if (window_buffer_sort->field == WINDOW_BUFFER_BY_SIZE)
		result = (*b)->size - (*a)->size;

	/* Use WINDOW_BUFFER_BY_NAME as default order and tie breaker. */
	if (result == 0)
		result = strcmp((*a)->name, (*b)->name);

	if (window_buffer_sort->reversed)
		result = -result;
	return (result);
}


// Source: window-buffer.c
// Lines 138-156
