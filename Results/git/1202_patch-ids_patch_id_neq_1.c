static int patch_id_neq(const void *cmpfn_data,
			const struct hashmap_entry *eptr,
			const struct hashmap_entry *entry_or_key,
			const void *unused_keydata)
{
	/* NEEDSWORK: const correctness? */
	struct diff_options *opt = (void *)cmpfn_data;
	struct patch_id *a, *b;

	a = container_of(eptr, struct patch_id, ent);
	b = container_of(entry_or_key, struct patch_id, ent);

	if (is_null_oid(&a->patch_id) &&
	    commit_patch_id(a->commit, opt, &a->patch_id, 0, 0))
		return error("Could not get patch ID for %s",
			oid_to_hex(&a->commit->object.oid));
	if (is_null_oid(&b->patch_id) &&
	    commit_patch_id(b->commit, opt, &b->patch_id, 0, 0))
		return error("Could not get patch ID for %s",
			oid_to_hex(&b->commit->object.oid));
	return !oideq(&a->patch_id, &b->patch_id);
}


// Source: patch-ids.c
// Lines 38-59
