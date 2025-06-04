static void filter_combine__finalize_omits(
	struct oidset *omits,
	void *filter_data)
{
	struct combine_filter_data *d = filter_data;
	size_t sub;

	for (sub = 0; sub < d->nr; sub++) {
		add_all(omits, &d->sub[sub].omits);
		oidset_clear(&d->sub[sub].omits);
	}
}


// Source: list-objects-filter.c
// Lines 723-734
