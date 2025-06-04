static void fill_origin_blob(struct diff_options *opt,
			     struct blame_origin *o, mmfile_t *file,
			     int *num_read_blob, int fill_fingerprints)
{
	if (!o->file.ptr) {
		enum object_type type;
		unsigned long file_size;

		(*num_read_blob)++;
		if (opt->flags.allow_textconv &&
		    textconv_object(opt->repo, o->path, o->mode,
				    &o->blob_oid, 1, &file->ptr, &file_size))
			;
		else
			file->ptr = read_object_file(&o->blob_oid, &type,
						     &file_size);
		file->size = file_size;

		if (!file->ptr)
			die("Cannot read blob %s for path %s",
			    oid_to_hex(&o->blob_oid),
			    o->path);
		o->file = *file;
	}


// Source: blame.c
// Lines 1017-1040
