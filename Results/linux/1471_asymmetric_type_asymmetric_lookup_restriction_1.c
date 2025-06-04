static struct key_restriction *asymmetric_lookup_restriction(
	const char *restriction)
{
	char *restrict_method;
	char *parse_buf;
	char *next;
	struct key_restriction *ret = ERR_PTR(-EINVAL);

	if (strcmp("builtin_trusted", restriction) == 0)
		return asymmetric_restriction_alloc(
			restrict_link_by_builtin_trusted, NULL);

	if (strcmp("builtin_and_secondary_trusted", restriction) == 0)
		return asymmetric_restriction_alloc(
			restrict_link_by_builtin_and_secondary_trusted, NULL);

	parse_buf = kstrndup(restriction, PAGE_SIZE, GFP_KERNEL);
	if (!parse_buf)
		return ERR_PTR(-ENOMEM);

	next = parse_buf;
	restrict_method = strsep(&next, ":");

	if ((strcmp(restrict_method, "key_or_keyring") == 0) && next) {
		char *key_text;
		key_serial_t serial;
		struct key *key;
		key_restrict_link_func_t link_fn =
			restrict_link_by_key_or_keyring;
		bool allow_null_key = false;

		key_text = strsep(&next, ":");

		if (next) {
			if (strcmp(next, "chain") != 0)
				goto out;

			link_fn = restrict_link_by_key_or_keyring_chain;
			allow_null_key = true;
		}

		if (kstrtos32(key_text, 0, &serial) < 0)
			goto out;

		if ((serial == 0) && allow_null_key) {
			key = NULL;
		} else {
			key = key_lookup(serial);
			if (IS_ERR(key)) {
				ret = ERR_CAST(key);
				goto out;
			}
		}

		ret = asymmetric_restriction_alloc(link_fn, key);
		if (IS_ERR(ret))
			key_put(key);
	}


// Source: asymmetric_type.c
// Lines 474-531
