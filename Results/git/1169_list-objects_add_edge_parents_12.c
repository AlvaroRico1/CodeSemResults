static void add_edge_parents(struct commit *commit,
			     struct rev_info *revs,
			     show_edge_fn show_edge,
			     struct oidset *set)
{
	struct commit_list *parents;

	for (parents = commit->parents; parents; parents = parents->next) {
		struct commit *parent = parents->item;
		struct tree *tree = get_commit_tree(parent);

		if (!tree)
			continue;

		oidset_insert(set, &tree->object.oid);

		if (!(parent->object.flags & UNINTERESTING))
			continue;
		tree->object.flags |= UNINTERESTING;

		if (revs->edge_hint && !(parent->object.flags & SHOWN)) {
			parent->object.flags |= SHOWN;
			show_edge(parent);
		}
	}


// Source: list-objects.c
// Lines 252-276
