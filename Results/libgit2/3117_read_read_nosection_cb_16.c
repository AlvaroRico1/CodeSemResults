static int read_nosection_cb(const git_config_entry *entry, void *payload) {
	int *seen = (int*)payload;
	if (strcmp(entry->name, "key") == 0) {
		(*seen)++;
	}
	return 0;
}


// Source: read.c
// Lines 933-939
