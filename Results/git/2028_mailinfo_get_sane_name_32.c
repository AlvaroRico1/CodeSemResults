static void get_sane_name(struct strbuf *out, struct strbuf *name, struct strbuf *email)
{
	struct strbuf *src = name;
	if (!name->len || 60 < name->len || strpbrk(name->buf, "@<>"))
		src = email;
	else if (name == out)
		return;
	strbuf_reset(out);
	strbuf_addbuf(out, src);
}


// Source: mailinfo.c
// Lines 19-28
