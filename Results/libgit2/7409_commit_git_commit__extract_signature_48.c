int git_commit__extract_signature(
	git_str *signature,
	git_str *signed_data,
	git_repository *repo,
	git_oid *commit_id,
	const char *field)
{
	git_odb_object *obj;
	git_odb *odb;
	const char *buf;
	const char *h, *eol;
	int error;

	git_str_clear(signature);
	git_str_clear(signed_data);

	if (!field)
		field = "gpgsig";

	if ((error = git_repository_odb__weakptr(&odb, repo)) < 0)
		return error;

	if ((error = git_odb_read(&obj, odb, commit_id)) < 0)
		return error;

	if (obj->cached.type != GIT_OBJECT_COMMIT) {
		git_error_set(GIT_ERROR_INVALID, "the requested type does not match the type in the ODB");
		error = GIT_ENOTFOUND;
		goto cleanup;
	}

	buf = git_odb_object_data(obj);

	while ((h = strchr(buf, '\n')) && h[1] != '\0') {
		h++;
		if (git__prefixcmp(buf, field)) {
			if (git_str_put(signed_data, buf, h - buf) < 0)
				return -1;

			buf = h;
			continue;
		}

		h = buf;
		h += strlen(field);
		eol = strchr(h, '\n');
		if (h[0] != ' ') {
			buf = h;
			continue;
		}
		if (!eol)
			goto malformed;

		h++; /* skip the SP */

		git_str_put(signature, h, eol - h);
		if (git_str_oom(signature))
			goto oom;

		/* If the next line starts with SP, it's multi-line, we must continue */
		while (eol[1] == ' ') {
			git_str_putc(signature, '\n');
			h = eol + 2;
			eol = strchr(h, '\n');
			if (!eol)
				goto malformed;

			git_str_put(signature, h, eol - h);
		}

		if (git_str_oom(signature))
			goto oom;

		error = git_str_puts(signed_data, eol+1);
		git_odb_object_free(obj);
		return error;
	}

	git_error_set(GIT_ERROR_OBJECT, "this commit is not signed");
	error = GIT_ENOTFOUND;
	goto cleanup;

malformed:
	git_error_set(GIT_ERROR_OBJECT, "malformed header");
	error = -1;
	goto cleanup;
oom:
	git_error_set_oom();
	error = -1;
	goto cleanup;

cleanup:
	git_odb_object_free(obj);
	git_str_clear(signature);
	git_str_clear(signed_data);
	return error;
}


// Source: commit.c
// Lines 787-883
