static void filter_sparse_free(void *filter_data)
{
	struct filter_sparse_data *d = filter_data;
	free(d->array_frame);
	free(d);
}


// Source: list-objects-filter.c
// Lines 514-519
