static size_t format_commit_one(struct strbuf *sb, /* in UTF-8 */
				const char *placeholder,
				void *context)
{
	struct format_commit_context *c = context;
	const struct commit *commit = c->commit;
	const char *msg = c->message;
	struct commit_list *p;
	const char *arg, *eol;
	size_t res;
	char **slot;

	/* these are independent of the commit */
	res = strbuf_expand_literal_cb(sb, placeholder, NULL);
	if (res)
		return res;

	switch (placeholder[0]) {
	case 'C':
		if (starts_with(placeholder + 1, "(auto)")) {
			c->auto_color = want_color(c->pretty_ctx->color);
			if (c->auto_color && sb->len)
				strbuf_addstr(sb, GIT_COLOR_RESET);
			return 7; /* consumed 7 bytes, "C(auto)" */
		} else {
			int ret = parse_color(sb, placeholder, c);
			if (ret)
				c->auto_color = 0;
			/*
			 * Otherwise, we decided to treat %C<unknown>
			 * as a literal string, and the previous
			 * %C(auto) is still valid.
			 */
			return ret;
		}
	case 'w':
		if (placeholder[1] == '(') {
			unsigned long width = 0, indent1 = 0, indent2 = 0;
			char *next;
			const char *start = placeholder + 2;
			const char *end = strchr(start, ')');
			if (!end)
				return 0;
			if (end > start) {
				width = strtoul(start, &next, 10);
				if (*next == ',') {
					indent1 = strtoul(next + 1, &next, 10);
					if (*next == ',') {
						indent2 = strtoul(next + 1,
								 &next, 10);
					}
				}
				if (*next != ')')
					return 0;
			}
			rewrap_message_tail(sb, c, width, indent1, indent2);
			return end - placeholder + 1;
		} else
			return 0;

	case '<':
	case '>':
		return parse_padding_placeholder(placeholder, c);
	}

	if (skip_prefix(placeholder, "(describe", &arg)) {
		struct child_process cmd = CHILD_PROCESS_INIT;
		struct strbuf out = STRBUF_INIT;
		struct strbuf err = STRBUF_INIT;
		struct pretty_print_describe_status *describe_status;

		describe_status = c->pretty_ctx->describe_status;
		if (describe_status) {
			if (!describe_status->max_invocations)
				return 0;
			describe_status->max_invocations--;
		}

		cmd.git_cmd = 1;
		strvec_push(&cmd.args, "describe");

		if (*arg == ':') {
			arg++;
			arg += parse_describe_args(arg, &cmd.args);
		}

		if (*arg != ')') {
			child_process_clear(&cmd);
			return 0;
		}

		strvec_push(&cmd.args, oid_to_hex(&commit->object.oid));
		pipe_command(&cmd, NULL, 0, &out, 0, &err, 0);
		strbuf_rtrim(&out);
		strbuf_addbuf(sb, &out);
		strbuf_release(&out);
		strbuf_release(&err);
		return arg - placeholder + 1;
	}

	/* these depend on the commit */
	if (!commit->object.parsed)
		parse_object(the_repository, &commit->object.oid);

	switch (placeholder[0]) {
	case 'H':		/* commit hash */
		strbuf_addstr(sb, diff_get_color(c->auto_color, DIFF_COMMIT));
		strbuf_addstr(sb, oid_to_hex(&commit->object.oid));
		strbuf_addstr(sb, diff_get_color(c->auto_color, DIFF_RESET));
		return 1;
	case 'h':		/* abbreviated commit hash */
		strbuf_addstr(sb, diff_get_color(c->auto_color, DIFF_COMMIT));
		strbuf_add_unique_abbrev(sb, &commit->object.oid,
					 c->pretty_ctx->abbrev);
		strbuf_addstr(sb, diff_get_color(c->auto_color, DIFF_RESET));
		return 1;
	case 'T':		/* tree hash */
		strbuf_addstr(sb, oid_to_hex(get_commit_tree_oid(commit)));
		return 1;
	case 't':		/* abbreviated tree hash */
		strbuf_add_unique_abbrev(sb,
					 get_commit_tree_oid(commit),
					 c->pretty_ctx->abbrev);
		return 1;
	case 'P':		/* parent hashes */
		for (p = commit->parents; p; p = p->next) {
			if (p != commit->parents)
				strbuf_addch(sb, ' ');
			strbuf_addstr(sb, oid_to_hex(&p->item->object.oid));
		}
		return 1;
	case 'p':		/* abbreviated parent hashes */
		for (p = commit->parents; p; p = p->next) {
			if (p != commit->parents)
				strbuf_addch(sb, ' ');
			strbuf_add_unique_abbrev(sb, &p->item->object.oid,
						 c->pretty_ctx->abbrev);
		}
		return 1;
	case 'm':		/* left/right/bottom */
		strbuf_addstr(sb, get_revision_mark(NULL, commit));
		return 1;
	case 'd':
		format_decorations(sb, commit, c->auto_color);
		return 1;
	case 'D':
		format_decorations_extended(sb, commit, c->auto_color, "", ", ", "");
		return 1;
	case 'S':		/* tag/branch like --source */
		if (!(c->pretty_ctx->rev && c->pretty_ctx->rev->sources))
			return 0;
		slot = revision_sources_at(c->pretty_ctx->rev->sources, commit);
		if (!(slot && *slot))
			return 0;
		strbuf_addstr(sb, *slot);
		return 1;
	case 'g':		/* reflog info */
		switch(placeholder[1]) {
		case 'd':	/* reflog selector */
		case 'D':
			if (c->pretty_ctx->reflog_info)
				get_reflog_selector(sb,
						    c->pretty_ctx->reflog_info,
						    &c->pretty_ctx->date_mode,
						    c->pretty_ctx->date_mode_explicit,
						    (placeholder[1] == 'd'));
			return 2;
		case 's':	/* reflog message */
			if (c->pretty_ctx->reflog_info)
				get_reflog_message(sb, c->pretty_ctx->reflog_info);
			return 2;
		case 'n':
		case 'N':
		case 'e':
		case 'E':
			return format_reflog_person(sb,
						    placeholder[1],
						    c->pretty_ctx->reflog_info,
						    &c->pretty_ctx->date_mode);
		}
		return 0;	/* unknown %g placeholder */
	case 'N':
		if (c->pretty_ctx->notes_message) {
			strbuf_addstr(sb, c->pretty_ctx->notes_message);
			return 1;
		}
		return 0;
	}

	if (placeholder[0] == 'G') {
		if (!c->signature_check.result)
			check_commit_signature(c->commit, &(c->signature_check));
		switch (placeholder[1]) {
		case 'G':
			if (c->signature_check.gpg_output)
				strbuf_addstr(sb, c->signature_check.gpg_output);
			break;
		case '?':
			switch (c->signature_check.result) {
			case 'G':
				switch (c->signature_check.trust_level) {
				case TRUST_UNDEFINED:
				case TRUST_NEVER:
					strbuf_addch(sb, 'U');
					break;
				default:
					strbuf_addch(sb, 'G');
					break;
				}
				break;
			case 'B':
			case 'E':
			case 'N':
			case 'X':
			case 'Y':
			case 'R':
				strbuf_addch(sb, c->signature_check.result);
			}
			break;
		case 'S':
			if (c->signature_check.signer)
				strbuf_addstr(sb, c->signature_check.signer);
			break;
		case 'K':
			if (c->signature_check.key)
				strbuf_addstr(sb, c->signature_check.key);
			break;
		case 'F':
			if (c->signature_check.fingerprint)
				strbuf_addstr(sb, c->signature_check.fingerprint);
			break;
		case 'P':
			if (c->signature_check.primary_key_fingerprint)
				strbuf_addstr(sb, c->signature_check.primary_key_fingerprint);
			break;
		case 'T':
			switch (c->signature_check.trust_level) {
			case TRUST_UNDEFINED:
				strbuf_addstr(sb, "undefined");
				break;
			case TRUST_NEVER:
				strbuf_addstr(sb, "never");
				break;
			case TRUST_MARGINAL:
				strbuf_addstr(sb, "marginal");
				break;
			case TRUST_FULLY:
				strbuf_addstr(sb, "fully");
				break;
			case TRUST_ULTIMATE:
				strbuf_addstr(sb, "ultimate");
				break;
			}
			break;
		default:
			return 0;
		}
		return 2;
	}

	/* For the rest we have to parse the commit header. */
	if (!c->commit_header_parsed) {
		msg = c->message =
			repo_logmsg_reencode(c->repository, commit,
					     &c->commit_encoding, "UTF-8");
		parse_commit_header(c);
	}

	switch (placeholder[0]) {
	case 'a':	/* author ... */
		return format_person_part(sb, placeholder[1],
				   msg + c->author.off, c->author.len,
				   &c->pretty_ctx->date_mode);
	case 'c':	/* committer ... */
		return format_person_part(sb, placeholder[1],
				   msg + c->committer.off, c->committer.len,
				   &c->pretty_ctx->date_mode);
	case 'e':	/* encoding */
		if (c->commit_encoding)
			strbuf_addstr(sb, c->commit_encoding);
		return 1;
	case 'B':	/* raw body */
		/* message_off is always left at the initial newline */
		strbuf_addstr(sb, msg + c->message_off + 1);
		return 1;
	}

	/* Now we need to parse the commit message. */
	if (!c->commit_message_parsed)
		parse_commit_message(c);

	switch (placeholder[0]) {
	case 's':	/* subject */
		format_subject(sb, msg + c->subject_off, " ");
		return 1;
	case 'f':	/* sanitized subject */
		eol = strchrnul(msg + c->subject_off, '\n');
		format_sanitized_subject(sb, msg + c->subject_off, eol - (msg + c->subject_off));
		return 1;
	case 'b':	/* body */
		strbuf_addstr(sb, msg + c->body_off);
		return 1;
	}

	if (skip_prefix(placeholder, "(trailers", &arg)) {
		struct process_trailer_options opts = PROCESS_TRAILER_OPTIONS_INIT;
		struct string_list filter_list = STRING_LIST_INIT_NODUP;
		struct strbuf sepbuf = STRBUF_INIT;
		struct strbuf kvsepbuf = STRBUF_INIT;
		size_t ret = 0;

		opts.no_divider = 1;

		if (*arg == ':') {
			arg++;
			if (format_set_trailers_options(&opts, &filter_list, &sepbuf, &kvsepbuf, &arg, NULL))
				goto trailer_out;
		}
		if (*arg == ')') {
			format_trailers_from_commit(sb, msg + c->subject_off, &opts);
			ret = arg - placeholder + 1;
		}
	trailer_out:
		string_list_clear(&filter_list, 0);
		strbuf_release(&sepbuf);
		return ret;
	}

	return 0;	/* unknown placeholder */
}


// Source: pretty.c
// Lines 1245-1574
