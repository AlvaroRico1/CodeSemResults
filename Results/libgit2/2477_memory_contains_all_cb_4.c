static int contains_all_cb(const git_config_entry *entry, void *payload)
{
	struct expected_entry *entries = (struct expected_entry *) payload;
	int i;

	for (i = 0; entries[i].name; i++) {
		if (strcmp(entries[i].name, entry->name) ||
		    strcmp(entries[i].value , entry->value))
			continue;

		if (entries[i].seen)
			cl_fail("Entry seen more than once");
		entries[i].seen = 1;
		return 0;
	}

	cl_fail("Unexpected entry");
	return -1;
}


// Source: memory.c
// Lines 31-49
