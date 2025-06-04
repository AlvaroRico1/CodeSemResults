static int read_oid_strbuf(struct merge_options *opt,
			   const struct object_id *oid,
			   struct strbuf *dst)
{
	void *buf;
	enum object_type type;
	unsigned long size;
	buf = read_object_file(oid, &type, &size);
	if (!buf)
		return err(opt, _("cannot read object %s"), oid_to_hex(oid));
	if (type != OBJ_BLOB) {
		free(buf);
		return err(opt, _("object %s is not a blob"), oid_to_hex(oid));
	}
	strbuf_attach(dst, buf, size, size + 1);
	return 0;
}

static int blob_unchanged(struct merge_options *opt,
			  const struct diff_filespec *o,
			  const struct diff_filespec *a,
			  int renormalize, const char *path)
{
	struct strbuf obuf = STRBUF_INIT;
	struct strbuf abuf = STRBUF_INIT;
	int ret = 0; /* assume changed for safety */
	struct index_state *idx = opt->repo->index;

	if (a->mode != o->mode)
		return 0;
	if (oideq(&o->oid, &a->oid))
		return 1;
	if (!renormalize)
		return 0;

	if (read_oid_strbuf(opt, &o->oid, &obuf) ||
	    read_oid_strbuf(opt, &a->oid, &abuf))
		goto error_return;
	/*
	 * Note: binary | is used so that both renormalizations are
	 * performed.  Comparison can be skipped if both files are
	 * unchanged since their sha1s have already been compared.
	 */
	if (renormalize_buffer(idx, path, obuf.buf, obuf.len, &obuf) |
	    renormalize_buffer(idx, path, abuf.buf, abuf.len, &abuf))
		ret = (obuf.len == abuf.len && !memcmp(obuf.buf, abuf.buf, obuf.len));

error_return:
	strbuf_release(&obuf);
	strbuf_release(&abuf);
	return ret;
}


// Source: merge-recursive.c
// Lines 3014-3065
