static int verify_object_in_tag(struct object_id *tagged_oid, int *tagged_type)
{
	int ret;
	enum object_type type;
	unsigned long size;
	void *buffer;
	const struct object_id *repl;

	buffer = read_object_file(tagged_oid, &type, &size);
	if (!buffer)
		die(_("could not read tagged object '%s'"),
		    oid_to_hex(tagged_oid));
	if (type != *tagged_type)
		die(_("object '%s' tagged as '%s', but is a '%s' type"),
		    oid_to_hex(tagged_oid),
		    type_name(*tagged_type), type_name(type));

	repl = lookup_replace_object(the_repository, tagged_oid);
	ret = check_object_signature(the_repository, repl,
				     buffer, size, type_name(*tagged_type));
	free(buffer);

	return ret;
}


// Source: mktag.c
// Lines 46-69
