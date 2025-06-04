void mark_edges_uninteresting(struct rev_info *revs,
			      show_edge_fn show_edge,
			      int sparse)
{
	struct commit_list *list;
	int i;

	if (sparse) {
		struct oidset set;
		oidset_init(&set, 16);

		for (list = revs->commits; list; list = list->next) {
			struct commit *commit = list->item;
			struct tree *tree = get_commit_tree(commit);

			if (commit->object.flags & UNINTERESTING)
				tree->object.flags |= UNINTERESTING;

			oidset_insert(&set, &tree->object.oid);
			add_edge_parents(commit, revs, show_edge, &set);
		}

		mark_trees_uninteresting_sparse(revs->repo, &set);
		oidset_clear(&set);
	} else {
		for (list = revs->commits; list; list = list->next) {
			struct commit *commit = list->item;
			if (commit->object.flags & UNINTERESTING) {
				mark_tree_uninteresting(revs->repo,
							get_commit_tree(commit));
				if (revs->edge_hint_aggressive && !(commit->object.flags & SHOWN)) {
					commit->object.flags |= SHOWN;
					show_edge(commit);
				}
				continue;
			}
			mark_edge_parents_uninteresting(commit, revs, show_edge);
		}
	}

	if (revs->edge_hint_aggressive) {
		for (i = 0; i < revs->cmdline.nr; i++) {
			struct object *obj = revs->cmdline.rev[i].item;
			struct commit *commit = (struct commit *)obj;
			if (obj->type != OBJ_COMMIT || !(obj->flags & UNINTERESTING))
				continue;
			mark_tree_uninteresting(revs->repo,
						get_commit_tree(commit));
			if (!(obj->flags & SHOWN)) {
				obj->flags |= SHOWN;
				show_edge(commit);
			}
		}
	}
}


// Source: list-objects.c
// Lines 279-333
