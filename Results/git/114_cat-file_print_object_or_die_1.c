static void print_object_or_die(struct batch_options *opt, struct expand_data *data)
{
	const struct object_id *oid = &data->oid;

	assert(data->info.typep);

	if (data->type == OBJ_BLOB) {
		if (opt->buffer_output)
			fflush(stdout);
		if (opt->cmdmode) {
			char *contents;
			unsigned long size;

			if (!data->rest)
				die("missing path for '%s'", oid_to_hex(oid));

			if (opt->cmdmode == 'w') {
				if (filter_object(data->rest, 0100644, oid,
						  &contents, &size))
					die("could not convert '%s' %s",
					    oid_to_hex(oid), data->rest);
			} else if (opt->cmdmode == 'c') {
				enum object_type type;
				if (!textconv_object(the_repository,
						     data->rest, 0100644, oid,
						     1, &contents, &size))
					contents = read_object_file(oid,
								    &type,
								    &size);
				if (!contents)
					die("could not convert '%s' %s",
					    oid_to_hex(oid), data->rest);
			} else
				BUG("invalid cmdmode: %c", opt->cmdmode);
			batch_write(opt, contents, size);
			free(contents);
		} else {
			stream_blob(oid);
		}


// Source: cat-file.c
// Lines 300-338
