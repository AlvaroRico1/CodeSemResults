static int which_parent(const struct object_id *oid, const struct commit *commit)
{
	int nth;
	const struct commit_list *parent;

	for (nth = 0, parent = commit->parents; parent; parent = parent->next) {
		if (oideq(&parent->item->object.oid, oid))
			return nth;
		nth++;
	}
	return -1;
}


// Source: log-tree.c
// Lines 529-540
