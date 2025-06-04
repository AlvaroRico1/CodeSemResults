input_key_build(void)
{
	struct input_key_entry	*ike, *new;
	u_int			 i, j;
	char			*data;
	key_code		 key;

	for (i = 0; i < nitems(input_key_defaults); i++) {
		ike = &input_key_defaults[i];
		if (~ike->key & KEYC_BUILD_MODIFIERS) {
			RB_INSERT(input_key_tree, &input_key_tree, ike);
			continue;
		}

		for (j = 2; j < nitems(input_key_modifiers); j++) {
			key = (ike->key & ~KEYC_BUILD_MODIFIERS);
			data = xstrdup(ike->data);
			data[strcspn(data, "_")] = '0' + j;

			new = xcalloc(1, sizeof *new);
			new->key = key|input_key_modifiers[j];
			new->data = data;
			RB_INSERT(input_key_tree, &input_key_tree, new);
		}
	}

	RB_FOREACH(ike, input_key_tree, &input_key_tree) {
		log_debug("%s: 0x%llx (%s) is %s", __func__, ike->key,
		    key_string_lookup_key(ike->key, 1), ike->data);
	}
}


// Source: input-keys.c
// Lines 358-388
