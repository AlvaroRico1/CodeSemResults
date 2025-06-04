static int parse_reuse_arg(const struct option *opt, const char *arg, int unset)
{
	struct note_data *d = opt->value;
	char *buf;
	struct object_id object;
	enum object_type type;
	unsigned long len;

	BUG_ON_OPT_NEG(unset);

	if (d->buf.len)
		strbuf_addch(&d->buf, '\n');

	if (get_oid(arg, &object))
		die(_("failed to resolve '%s' as a valid ref."), arg);
	if (!(buf = read_object_file(&object, &type, &len)))
		die(_("failed to read object '%s'."), arg);
	if (type != OBJ_BLOB) {
		free(buf);
		die(_("cannot read note data from non-blob object '%s'."), arg);
	}
	strbuf_add(&d->buf, buf, len);
	free(buf);

	d->given = 1;
	return 0;
}


// Source: notes.c
// Lines 247-273
