int git_commit__header_field(
	git_str *out,
	const git_commit *commit,
	const char *field)
{
	const char *eol, *buf = commit->raw_header;

	git_str_clear(out);

	while ((eol = strchr(buf, '\n'))) {
		/* We can skip continuations here */
		if (buf[0] == ' ') {
			buf = eol + 1;
			continue;
		}

		/* Skip until we find the field we're after */
		if (git__prefixcmp(buf, field)) {
			buf = eol + 1;
			continue;
		}

		buf += strlen(field);
		/* Check that we're not matching a prefix but the field itself */
		if (buf[0] != ' ') {
			buf = eol + 1;
			continue;
		}

		buf++; /* skip the SP */

		git_str_put(out, buf, eol - buf);
		if (git_str_oom(out))
			goto oom;

		/* If the next line starts with SP, it's multi-line, we must continue */
		while (eol[1] == ' ') {
			git_str_putc(out, '\n');
			buf = eol + 2;
			eol = strchr(buf, '\n');
			if (!eol)
				goto malformed;

			git_str_put(out, buf, eol - buf);
		}

		if (git_str_oom(out))
			goto oom;

		return 0;
	}

	git_error_set(GIT_ERROR_OBJECT, "no such field '%s'", field);
	return GIT_ENOTFOUND;

malformed:
	git_error_set(GIT_ERROR_OBJECT, "malformed header");
	return -1;
oom:
	git_error_set_oom();
	return -1;
}


// Source: commit.c
// Lines 701-762
