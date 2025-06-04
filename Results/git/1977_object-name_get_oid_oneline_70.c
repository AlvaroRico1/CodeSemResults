static int get_oid_oneline(struct repository *r,
			   const char *prefix, struct object_id *oid,
			   struct commit_list *list)
{
	struct commit_list *backup = NULL, *l;
	int found = 0;
	int negative = 0;
	regex_t regex;

	if (prefix[0] == '!') {
		prefix++;

		if (prefix[0] == '-') {
			prefix++;
			negative = 1;
		} else if (prefix[0] != '!') {
			return -1;
		}
	}

	if (regcomp(&regex, prefix, REG_EXTENDED))
		return -1;

	for (l = list; l; l = l->next) {
		l->item->object.flags |= ONELINE_SEEN;
		commit_list_insert(l->item, &backup);
	}
	while (list) {
		const char *p, *buf;
		struct commit *commit;
		int matches;

		commit = pop_most_recent_commit(&list, ONELINE_SEEN);
		if (!parse_object(r, &commit->object.oid))
			continue;
		buf = get_commit_buffer(commit, NULL);
		p = strstr(buf, "\n\n");
		matches = negative ^ (p && !regexec(&regex, p + 2, 0, NULL, 0));
		unuse_commit_buffer(commit, buf);

		if (matches) {
			oidcpy(oid, &commit->object.oid);
			found = 1;
			break;
		}
	}


// Source: object-name.c
// Lines 1230-1275
