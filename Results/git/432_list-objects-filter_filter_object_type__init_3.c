static void filter_object_type__init(
	struct list_objects_filter_options *filter_options,
	struct filter *filter)
{
	struct filter_object_type_data *d = xcalloc(1, sizeof(*d));
	d->object_type = filter_options->object_type;

	filter->filter_data = d;
	filter->filter_object_fn = filter_object_type;
	filter->free_fn = free;
}


// Source: list-objects-filter.c
// Lines 611-621
