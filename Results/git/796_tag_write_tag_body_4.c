static void write_tag_body(int fd, const struct object_id *oid)
{
	unsigned long size;
	enum object_type type;
	char *buf, *sp, *orig;
	struct strbuf payload = STRBUF_INIT;
	struct strbuf signature = STRBUF_INIT;

	orig = buf = read_object_file(oid, &type, &size);
	if (!buf)
		return;
	if (parse_signature(buf, size, &payload, &signature)) {
		buf = payload.buf;
		size = payload.len;
	}
	/* skip header */
	sp = strstr(buf, "\n\n");

	if (!sp || !size || type != OBJ_TAG) {
		free(buf);
		return;
	}
	sp += 2; /* skip the 2 LFs */
	write_or_die(fd, sp, buf + size - sp);

	free(orig);
	strbuf_release(&payload);
	strbuf_release(&signature);
}


// Source: tag.c
// Lines 208-236
