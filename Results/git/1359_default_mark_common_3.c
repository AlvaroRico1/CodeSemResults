static void mark_common(struct negotiation_state *ns, struct commit *commit,
		int ancestors_only, int dont_parse)
{
	if (commit != NULL && !(commit->object.flags & COMMON)) {
		struct object *o = (struct object *)commit;

		if (!ancestors_only)
			o->flags |= COMMON;

		if (!(o->flags & SEEN))
			rev_list_push(ns, commit, SEEN);
		else {
			struct commit_list *parents;

			if (!ancestors_only && !(o->flags & POPPED))
				ns->non_common_revs--;
			if (!o->parsed && !dont_parse)
				if (parse_commit(commit))
					return;

			for (parents = commit->parents;
					parents;
					parents = parents->next)
				mark_common(ns, parents->item, 0,
					    dont_parse);
		}


// Source: default.c
// Lines 54-79
