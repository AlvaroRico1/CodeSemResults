int ls_refs(struct repository *r, struct packet_reader *request)
{
	struct ls_refs_data data;

	memset(&data, 0, sizeof(data));
	strvec_init(&data.prefixes);
	strbuf_init(&data.buf, 0);

	ensure_config_read();
	git_config(ls_refs_config, NULL);

	while (packet_reader_read(request) == PACKET_READ_NORMAL) {
		const char *arg = request->line;
		const char *out;

		if (!strcmp("peel", arg))
			data.peel = 1;
		else if (!strcmp("symrefs", arg))
			data.symrefs = 1;
		else if (skip_prefix(arg, "ref-prefix ", &out)) {
			if (data.prefixes.nr < TOO_MANY_PREFIXES)
				strvec_push(&data.prefixes, out);
		}
		else if (!strcmp("unborn", arg))
			data.unborn = allow_unborn;
		else
			die(_("unexpected line: '%s'"), arg);
	}


// Source: ls-refs.c
// Lines 148-175
