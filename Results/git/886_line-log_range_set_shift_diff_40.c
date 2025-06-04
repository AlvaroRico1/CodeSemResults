static void range_set_shift_diff(struct range_set *out,
				 struct range_set *rs,
				 struct diff_ranges *diff)
{
	unsigned int i, j = 0;
	long offset = 0;
	struct range *src = rs->ranges;
	struct range *target = diff->target.ranges;
	struct range *parent = diff->parent.ranges;

	for (i = 0; i < rs->nr; i++) {
		while (j < diff->target.nr && src[i].start >= target[j].start) {
			offset += (parent[j].end-parent[j].start)
				- (target[j].end-target[j].start);
			j++;
		}
		range_set_append(out, src[i].start+offset, src[i].end+offset);
	}
}


// Source: line-log.c
// Lines 426-444
