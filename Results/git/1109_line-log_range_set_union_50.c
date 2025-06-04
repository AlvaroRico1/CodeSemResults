static void range_set_union(struct range_set *out,
			     struct range_set *a, struct range_set *b)
{
	unsigned int i = 0, j = 0;
	struct range *ra = a->ranges;
	struct range *rb = b->ranges;
	/* cannot make an alias of out->ranges: it may change during grow */

	assert(out->nr == 0);
	while (i < a->nr || j < b->nr) {
		struct range *new_range;
		if (i < a->nr && j < b->nr) {
			if (ra[i].start < rb[j].start)
				new_range = &ra[i++];
			else if (ra[i].start > rb[j].start)
				new_range = &rb[j++];
			else if (ra[i].end < rb[j].end)
				new_range = &ra[i++];
			else
				new_range = &rb[j++];
		} else if (i < a->nr)      /* b exhausted */
			new_range = &ra[i++];
		else                       /* a exhausted */
			new_range = &rb[j++];
		if (new_range->start == new_range->end)
			; /* empty range */
		else if (!out->nr || out->ranges[out->nr-1].end < new_range->start) {
			range_set_grow(out, 1);
			out->ranges[out->nr].start = new_range->start;
			out->ranges[out->nr].end = new_range->end;
			out->nr++;
		} else if (out->ranges[out->nr-1].end < new_range->end) {
			out->ranges[out->nr-1].end = new_range->end;
		}
	}
}


// Source: line-log.c
// Lines 145-180
