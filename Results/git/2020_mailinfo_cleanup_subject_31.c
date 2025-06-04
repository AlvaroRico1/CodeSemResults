static void cleanup_subject(struct mailinfo *mi, struct strbuf *subject)
{
	size_t at = 0;

	while (at < subject->len) {
		char *pos;
		size_t remove;

		switch (subject->buf[at]) {
		case 'r': case 'R':
			if (subject->len <= at + 3)
				break;
			if ((subject->buf[at + 1] == 'e' ||
			     subject->buf[at + 1] == 'E') &&
			    subject->buf[at + 2] == ':') {
				strbuf_remove(subject, at, 3);
				continue;
			}
			at++;
			break;
		case ' ': case '\t': case ':':
			strbuf_remove(subject, at, 1);
			continue;
		case '[':
			pos = strchr(subject->buf + at, ']');
			if (!pos)
				break;
			remove = pos - subject->buf + at + 1;
			if (!mi->keep_non_patch_brackets_in_subject ||
			    (7 <= remove &&
			     memmem(subject->buf + at, remove, "PATCH", 5)))
				strbuf_remove(subject, at, remove);
			else {
				at += remove;
				/*
				 * If the input had a space after the ], keep
				 * it.  We don't bother with finding the end of
				 * the space, since we later normalize it
				 * anyway.
				 */
				if (isspace(subject->buf[at]))
					at += 1;
			}
			continue;
		}
		break;
	}


// Source: mailinfo.c
// Lines 293-339
