void log_write_email_headers(struct rev_info *opt, struct commit *commit,
			     const char **extra_headers_p,
			     int *need_8bit_cte_p,
			     int maybe_multipart)
{
	const char *extra_headers = opt->extra_headers;
	const char *name = oid_to_hex(opt->zero_commit ?
				      null_oid() : &commit->object.oid);

	*need_8bit_cte_p = 0; /* unknown */

	fprintf(opt->diffopt.file, "From %s Mon Sep 17 00:00:00 2001\n", name);
	graph_show_oneline(opt->graph);
	if (opt->message_id) {
		fprintf(opt->diffopt.file, "Message-Id: <%s>\n", opt->message_id);
		graph_show_oneline(opt->graph);
	}
	if (opt->ref_message_ids && opt->ref_message_ids->nr > 0) {
		int i, n;
		n = opt->ref_message_ids->nr;
		fprintf(opt->diffopt.file, "In-Reply-To: <%s>\n", opt->ref_message_ids->items[n-1].string);
		for (i = 0; i < n; i++)
			fprintf(opt->diffopt.file, "%s<%s>\n", (i > 0 ? "\t" : "References: "),
			       opt->ref_message_ids->items[i].string);
		graph_show_oneline(opt->graph);
	}
	if (opt->mime_boundary && maybe_multipart) {
		static struct strbuf subject_buffer = STRBUF_INIT;
		static struct strbuf buffer = STRBUF_INIT;
		struct strbuf filename =  STRBUF_INIT;
		*need_8bit_cte_p = -1; /* NEVER */

		strbuf_reset(&subject_buffer);
		strbuf_reset(&buffer);

		strbuf_addf(&subject_buffer,
			 "%s"
			 "MIME-Version: 1.0\n"
			 "Content-Type: multipart/mixed;"
			 " boundary=\"%s%s\"\n"
			 "\n"
			 "This is a multi-part message in MIME "
			 "format.\n"
			 "--%s%s\n"
			 "Content-Type: text/plain; "
			 "charset=UTF-8; format=fixed\n"
			 "Content-Transfer-Encoding: 8bit\n\n",
			 extra_headers ? extra_headers : "",
			 mime_boundary_leader, opt->mime_boundary,
			 mime_boundary_leader, opt->mime_boundary);
		extra_headers = subject_buffer.buf;

		if (opt->numbered_files)
			strbuf_addf(&filename, "%d", opt->nr);
		else
			fmt_output_commit(&filename, commit, opt);
		strbuf_addf(&buffer,
			 "\n--%s%s\n"
			 "Content-Type: text/x-patch;"
			 " name=\"%s\"\n"
			 "Content-Transfer-Encoding: 8bit\n"
			 "Content-Disposition: %s;"
			 " filename=\"%s\"\n\n",
			 mime_boundary_leader, opt->mime_boundary,
			 filename.buf,
			 opt->no_inline ? "attachment" : "inline",
			 filename.buf);
		opt->diffopt.stat_sep = buffer.buf;
		strbuf_release(&filename);
	}
	*extra_headers_p = extra_headers;
}


// Source: log-tree.c
// Lines 417-488
