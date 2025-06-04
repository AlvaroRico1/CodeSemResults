static void filter_combine__free(void *filter_data)
{
	struct combine_filter_data *d = filter_data;
	size_t sub;
	for (sub = 0; sub < d->nr; sub++) {
		list_objects_filter__free(d->sub[sub].filter);
		oidset_clear(&d->sub[sub].seen);
		if (d->sub[sub].omits.set.size)
			BUG("expected oidset to be cleared already");
	}
	free(d->sub);
}


// Source: list-objects-filter.c
// Lines 701-712
