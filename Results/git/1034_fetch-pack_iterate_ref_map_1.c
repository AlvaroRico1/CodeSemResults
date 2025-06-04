static const struct object_id *iterate_ref_map(void *cb_data)
{
	struct ref **rm = cb_data;
	struct ref *ref = *rm;

	if (!ref)
		return NULL;
	*rm = ref->next;
	return &ref->old_oid;
}

struct ref *fetch_pack(struct fetch_pack_args *args,
		       int fd[],
		       const struct ref *ref,
		       struct ref **sought, int nr_sought,
		       struct oid_array *shallow,
		       struct string_list *pack_lockfiles,
		       enum protocol_version version)
{
	struct ref *ref_cpy;
	struct shallow_info si;
	struct oid_array shallows_scratch = OID_ARRAY_INIT;

	fetch_pack_setup();
	if (nr_sought)
		nr_sought = remove_duplicates_in_refs(sought, nr_sought);

	if (version != protocol_v2 && !ref) {
		packet_flush(fd[1]);
		die(_("no matching remote head"));
	}
	if (version == protocol_v2) {
		if (shallow->nr)
			BUG("Protocol V2 does not provide shallows at this point in the fetch");
		memset(&si, 0, sizeof(si));
		ref_cpy = do_fetch_pack_v2(args, fd, ref, sought, nr_sought,
					   &shallows_scratch, &si,
					   pack_lockfiles);
	} else {
		prepare_shallow_info(&si, shallow);
		ref_cpy = do_fetch_pack(args, fd, ref, sought, nr_sought,
					&si, pack_lockfiles);
	}


// Source: fetch-pack.c
// Lines 1920-1962
