static void loose_fill_ref_dir(struct ref_store *ref_store,
			       struct ref_dir *dir, const char *dirname)
{
	struct files_ref_store *refs =
		files_downcast(ref_store, REF_STORE_READ, "fill_ref_dir");
	DIR *d;
	struct dirent *de;
	int dirnamelen = strlen(dirname);
	struct strbuf refname;
	struct strbuf path = STRBUF_INIT;
	size_t path_baselen;

	files_ref_path(refs, &path, dirname);
	path_baselen = path.len;

	d = opendir(path.buf);
	if (!d) {
		strbuf_release(&path);
		return;
	}

	strbuf_init(&refname, dirnamelen + 257);
	strbuf_add(&refname, dirname, dirnamelen);

	while ((de = readdir(d)) != NULL) {
		struct object_id oid;
		struct stat st;
		int flag;

		if (de->d_name[0] == '.')
			continue;
		if (ends_with(de->d_name, ".lock"))
			continue;
		strbuf_addstr(&refname, de->d_name);
		strbuf_addstr(&path, de->d_name);
		if (stat(path.buf, &st) < 0) {
			; /* silently ignore */
		} else if (S_ISDIR(st.st_mode)) {
			strbuf_addch(&refname, '/');
			add_entry_to_dir(dir,
					 create_dir_entry(dir->cache, refname.buf,
							  refname.len));
		} else {
			if (!refs_resolve_ref_unsafe(&refs->base,
						     refname.buf,
						     RESOLVE_REF_READING,
						     &oid, &flag)) {
				oidclr(&oid);
				flag |= REF_ISBROKEN;
			} else if (is_null_oid(&oid)) {
				/*
				 * It is so astronomically unlikely
				 * that null_oid is the OID of an
				 * actual object that we consider its
				 * appearance in a loose reference
				 * file to be repo corruption
				 * (probably due to a software bug).
				 */
				flag |= REF_ISBROKEN;
			}

			if (check_refname_format(refname.buf,
						 REFNAME_ALLOW_ONELEVEL)) {
				if (!refname_is_safe(refname.buf))
					die("loose refname is dangerous: %s", refname.buf);
				oidclr(&oid);
				flag |= REF_BAD_NAME | REF_ISBROKEN;
			}
			add_entry_to_dir(dir,
					 create_ref_entry(refname.buf, &oid, flag));
		}
		strbuf_setlen(&refname, dirnamelen);
		strbuf_setlen(&path, path_baselen);
	}


// Source: files-backend.c
// Lines 240-313
