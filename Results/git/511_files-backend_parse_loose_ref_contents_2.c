int parse_loose_ref_contents(const char *buf, struct object_id *oid,
			     struct strbuf *referent, unsigned int *type)
{
	const char *p;
	if (skip_prefix(buf, "ref:", &buf)) {
		while (isspace(*buf))
			buf++;

		strbuf_reset(referent);
		strbuf_addstr(referent, buf);
		*type |= REF_ISSYMREF;
		return 0;
	}

	/*
	 * FETCH_HEAD has additional data after the sha.
	 */
	if (parse_oid_hex(buf, oid, &p) ||
	    (*p != '\0' && !isspace(*p))) {
		*type |= REF_ISBROKEN;
		errno = EINVAL;
		return -1;
	}
	return 0;
}


// Source: files-backend.c
// Lines 467-491
