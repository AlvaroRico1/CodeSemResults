static int commit_rewrite_person(struct strbuf *buf, const char *what, struct string_list *mailmap)
{
	char *person, *endp;
	size_t len, namelen, maillen;
	const char *name;
	const char *mail;
	struct ident_split ident;

	person = strstr(buf->buf, what);
	if (!person)
		return 0;

	person += strlen(what);
	endp = strchr(person, '\n');
	if (!endp)
		return 0;

	len = endp - person;

	if (split_ident_line(&ident, person, len))
		return 0;

	mail = ident.mail_begin;
	maillen = ident.mail_end - ident.mail_begin;
	name = ident.name_begin;
	namelen = ident.name_end - ident.name_begin;

	if (map_user(mailmap, &mail, &maillen, &name, &namelen)) {
		struct strbuf namemail = STRBUF_INIT;

		strbuf_addf(&namemail, "%.*s <%.*s>",
			    (int)namelen, name, (int)maillen, mail);

		strbuf_splice(buf, ident.name_begin - buf->buf,
			      ident.mail_end - ident.name_begin + 1,
			      namemail.buf, namemail.len);

		strbuf_release(&namemail);

		return 1;
	}

	return 0;
}


// Source: revision.c
// Lines 3684-3727
