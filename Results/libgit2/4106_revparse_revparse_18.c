static int revparse(
	git_object **object_out,
	git_reference **reference_out,
	size_t *identifier_len_out,
	git_repository *repo,
	const char *spec)
{
	size_t pos = 0, identifier_len = 0;
	int error = -1, n;
	git_str buf = GIT_STR_INIT;

	git_reference *reference = NULL;
	git_object *base_rev = NULL;

	bool should_return_reference = true;

	GIT_ASSERT_ARG(object_out);
	GIT_ASSERT_ARG(reference_out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(spec);

	*object_out = NULL;
	*reference_out = NULL;

	while (spec[pos]) {
		switch (spec[pos]) {
		case '^':
			should_return_reference = false;

			if ((error = ensure_base_rev_loaded(&base_rev, &reference, spec, identifier_len, repo, false)) < 0)
				goto cleanup;

			if (spec[pos+1] == '{') {
				git_object *temp_object = NULL;

				if ((error = extract_curly_braces_content(&buf, spec, &pos)) < 0)
					goto cleanup;

				if ((error = handle_caret_curly_syntax(&temp_object, base_rev, git_str_cstr(&buf))) < 0)
					goto cleanup;

				git_object_free(base_rev);
				base_rev = temp_object;
			} else {
				git_object *temp_object = NULL;

				if ((error = extract_how_many(&n, spec, &pos)) < 0)
					goto cleanup;

				if ((error = handle_caret_parent_syntax(&temp_object, base_rev, n)) < 0)
					goto cleanup;

				git_object_free(base_rev);
				base_rev = temp_object;
			}
			break;

		case '~':
		{
			git_object *temp_object = NULL;

			should_return_reference = false;

			if ((error = extract_how_many(&n, spec, &pos)) < 0)
				goto cleanup;

			if ((error = ensure_base_rev_loaded(&base_rev, &reference, spec, identifier_len, repo, false)) < 0)
				goto cleanup;

			if ((error = handle_linear_syntax(&temp_object, base_rev, n)) < 0)
				goto cleanup;

			git_object_free(base_rev);
			base_rev = temp_object;
			break;
		}

		case ':':
		{
			git_object *temp_object = NULL;

			should_return_reference = false;

			if ((error = extract_path(&buf, spec, &pos)) < 0)
				goto cleanup;

			if (any_left_hand_identifier(base_rev, reference, identifier_len)) {
				if ((error = ensure_base_rev_loaded(&base_rev, &reference, spec, identifier_len, repo, true)) < 0)
					goto cleanup;

				if ((error = handle_colon_syntax(&temp_object, base_rev, git_str_cstr(&buf))) < 0)
					goto cleanup;
			} else {
				if (*git_str_cstr(&buf) == '/') {
					if ((error = handle_grep_syntax(&temp_object, repo, NULL, git_str_cstr(&buf) + 1)) < 0)
						goto cleanup;
				} else {

					/*
					 * TODO: support merge-stage path lookup (":2:Makefile")
					 * and plain index blob lookup (:i-am/a/blob)
					 */
					git_error_set(GIT_ERROR_INVALID, "unimplemented");
					error = GIT_ERROR;
					goto cleanup;
				}
			}

			git_object_free(base_rev);
			base_rev = temp_object;
			break;
		}


// Source: revparse.c
// Lines 673-784
