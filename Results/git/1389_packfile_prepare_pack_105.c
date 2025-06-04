static void prepare_pack(const char *full_name, size_t full_name_len,
			 const char *file_name, void *_data)
{
	struct prepare_pack_data *data = (struct prepare_pack_data *)_data;
	struct packed_git *p;
	size_t base_len = full_name_len;

	if (strip_suffix_mem(full_name, &base_len, ".idx") &&
	    !(data->m && midx_contains_pack(data->m, file_name))) {
		struct hashmap_entry hent;
		char *pack_name = xstrfmt("%.*s.pack", (int)base_len, full_name);
		unsigned int hash = strhash(pack_name);
		hashmap_entry_init(&hent, hash);

		/* Don't reopen a pack we already have. */
		if (!hashmap_get(&data->r->objects->pack_map, &hent, pack_name)) {
			p = add_packed_git(full_name, full_name_len, data->local);
			if (p)
				install_packed_git(data->r, p);
		}
		free(pack_name);
	}

	if (!report_garbage)
		return;

	if (!strcmp(file_name, "multi-pack-index"))
		return;
	if (starts_with(file_name, "multi-pack-index") &&
	    (ends_with(file_name, ".bitmap") || ends_with(file_name, ".rev")))
		return;
	if (ends_with(file_name, ".idx") ||
	    ends_with(file_name, ".rev") ||
	    ends_with(file_name, ".pack") ||
	    ends_with(file_name, ".bitmap") ||
	    ends_with(file_name, ".keep") ||
	    ends_with(file_name, ".promisor"))
		string_list_append(data->garbage, full_name);
	else
		report_garbage(PACKDIR_FILE_GARBAGE, full_name);
}


// Source: packfile.c
// Lines 835-875
