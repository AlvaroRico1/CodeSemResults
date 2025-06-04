void for_each_alternate_ref(alternate_ref_fn fn, void *data)
{
	struct alternate_refs_data cb;
	cb.fn = fn;
	cb.data = data;
	foreach_alt_odb(refs_from_alternate_cb, &cb);
}


// Source: object-file.c
// Lines 867-873
