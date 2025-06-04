static int merge_changes(struct notes_merge_options *o,
			 struct notes_merge_pair *changes, int *num_changes,
			 struct notes_tree *t)
{
	int i, conflicts = 0;

	trace_printf("\tmerge_changes(num_changes = %i)\n", *num_changes);
	for (i = 0; i < *num_changes; i++) {
		struct notes_merge_pair *p = changes + i;
		trace_printf("\t\t%.7s: %.7s -> %.7s/%.7s\n",
		       oid_to_hex(&p->obj), oid_to_hex(&p->base),
		       oid_to_hex(&p->local),
		       oid_to_hex(&p->remote));

		if (oideq(&p->base, &p->remote)) {
			/* no remote change; nothing to do */
			trace_printf("\t\t\tskipping (no remote change)\n");
		} else if (oideq(&p->local, &p->remote)) {
			/* same change in local and remote; nothing to do */
			trace_printf("\t\t\tskipping (local == remote)\n");
		} else if (oideq(&p->local, &uninitialized) ||
			   oideq(&p->local, &p->base)) {
			/* no local change; adopt remote change */
			trace_printf("\t\t\tno local change, adopted remote\n");
			if (add_note(t, &p->obj, &p->remote,
				     combine_notes_overwrite))
				BUG("combine_notes_overwrite failed");
		} else {
			/* need file-level merge between local and remote */
			trace_printf("\t\t\tneed content-level merge\n");
			conflicts += merge_one_change(o, p, t);
		}
	}


// Source: notes-merge.c
// Lines 473-505
