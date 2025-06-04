static int hashsig_add_hashes(
	git_hashsig *sig,
	const uint8_t *data,
	size_t size,
	hashsig_in_progress *prog)
{
	const uint8_t *scan = data, *end = data + size;
	hashsig_state state = HASHSIG_HASH_START;
	int use_ignores = prog->use_ignores, len;
	uint8_t ch;

	while (scan < end) {
		state = HASHSIG_HASH_START;

		for (len = 0; scan < end && len < HASHSIG_MAX_RUN; ) {
			ch = *scan;

			if (use_ignores)
				for (; scan < end && git__isspace_nonlf(ch); ch = *scan)
					++scan;
			else if (sig->opt &
					 (GIT_HASHSIG_IGNORE_WHITESPACE | GIT_HASHSIG_SMART_WHITESPACE))
				for (; scan < end && ch == '\r'; ch = *scan)
					++scan;

			/* peek at next character to decide what to do next */
			if (sig->opt & GIT_HASHSIG_SMART_WHITESPACE)
				use_ignores = (ch == '\n');

			if (scan >= end)
				break;
			++scan;

			/* check run terminator */
			if (ch == '\n' || ch == '\0') {
				sig->lines++;
				break;
			}

			++len;
			HASHSIG_HASH_MIX(state, ch);
		}

		if (len > 0) {
			hashsig_heap_insert(&sig->mins, (hashsig_t)state);
			hashsig_heap_insert(&sig->maxs, (hashsig_t)state);

			while (scan < end && (*scan == '\n' || !*scan))
				++scan;
		}
	}

	prog->use_ignores = use_ignores;

	return 0;
}


// Source: hashsig.c
// Lines 160-215
