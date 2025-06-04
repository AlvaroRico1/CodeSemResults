static struct repository *open_submodule(const char *path)
{
	struct strbuf sb = STRBUF_INIT;
	struct repository *out = xmalloc(sizeof(*out));

	if (submodule_to_gitdir(&sb, path) || repo_init(out, sb.buf, NULL)) {
		strbuf_release(&sb);
		free(out);
		return NULL;
	}


// Source: submodule.c
// Lines 531-540
