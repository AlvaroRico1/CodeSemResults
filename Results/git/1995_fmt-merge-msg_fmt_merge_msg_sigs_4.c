static void fmt_merge_msg_sigs(struct strbuf *out)
{
	int i, tag_number = 0, first_tag = 0;
	struct strbuf tagbuf = STRBUF_INIT;

	for (i = 0; i < origins.nr; i++) {
		struct object_id *oid = origins.items[i].util;
		enum object_type type;
		unsigned long size;
		char *buf = read_object_file(oid, &type, &size);
		char *origbuf = buf;
		unsigned long len = size;
		struct signature_check sigc = { NULL };
		struct strbuf payload = STRBUF_INIT, sig = STRBUF_INIT;

		if (!buf || type != OBJ_TAG)
			goto next;

		if (!parse_signature(buf, size, &payload, &sig))
			;/* merely annotated */
		else {
			buf = payload.buf;
			len = payload.len;
			if (check_signature(payload.buf, payload.len, sig.buf,
					 sig.len, &sigc) &&
				!sigc.gpg_output)
				strbuf_addstr(&sig, "gpg verification failed.\n");
			else
				strbuf_addstr(&sig, sigc.gpg_output);
		}
		signature_check_clear(&sigc);

		if (!tag_number++) {
			fmt_tag_signature(&tagbuf, &sig, buf, len);
			first_tag = i;
		} else {
			if (tag_number == 2) {
				struct strbuf tagline = STRBUF_INIT;
				strbuf_addch(&tagline, '\n');
				strbuf_add_commented_lines(&tagline,
						origins.items[first_tag].string,
						strlen(origins.items[first_tag].string));
				strbuf_insert(&tagbuf, 0, tagline.buf,
					      tagline.len);
				strbuf_release(&tagline);
			}
			strbuf_addch(&tagbuf, '\n');
			strbuf_add_commented_lines(&tagbuf,
					origins.items[i].string,
					strlen(origins.items[i].string));
			fmt_tag_signature(&tagbuf, &sig, buf, len);
		}
		strbuf_release(&payload);
		strbuf_release(&sig);
	next:
		free(origbuf);
	}


// Source: fmt-merge-msg.c
// Lines 507-563
