static void prepare_packed_git_one(struct repository *r, char *objdir, int local)
{
	struct prepare_pack_data data;
	struct string_list garbage = STRING_LIST_INIT_DUP;

	data.m = r->objects->multi_pack_index;

	/* look for the multi-pack-index for this object directory */
	while (data.m && strcmp(data.m->object_dir, objdir))
		data.m = data.m->next;

	data.r = r;
	data.garbage = &garbage;
	data.local = local;

	for_each_file_in_pack_dir(objdir, prepare_pack, &data);

	report_pack_garbage(data.garbage);
	string_list_clear(data.garbage, 0);
}


// Source: packfile.c
// Lines 877-896
