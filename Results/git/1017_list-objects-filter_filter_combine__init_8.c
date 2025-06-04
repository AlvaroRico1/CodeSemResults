static void filter_combine__init(
	struct list_objects_filter_options *filter_options,
	struct filter* filter)
{
	struct combine_filter_data *d = xcalloc(1, sizeof(*d));
	size_t sub;

	d->nr = filter_options->sub_nr;
	CALLOC_ARRAY(d->sub, d->nr);
	for (sub = 0; sub < d->nr; sub++)
		d->sub[sub].filter = list_objects_filter__init(
			filter->omits ? &d->sub[sub].omits : NULL,
			&filter_options->sub[sub]);

	filter->filter_data = d;
	filter->filter_object_fn = filter_combine;
	filter->free_fn = filter_combine__free;
	filter->finalize_omits_fn = filter_combine__finalize_omits;
}


// Source: list-objects-filter.c
// Lines 736-754
