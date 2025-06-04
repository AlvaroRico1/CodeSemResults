static int should_include(struct commit *commit, void *_data)
{
	struct include_data *data = _data;
	int bitmap_pos;

	bitmap_pos = bitmap_position(data->bitmap_git, &commit->object.oid);
	if (bitmap_pos < 0)
		bitmap_pos = ext_index_add_object(data->bitmap_git,
						  (struct object *)commit,
						  NULL);

	if (!add_to_include_set(data->bitmap_git, data, commit, bitmap_pos)) {
		struct commit_list *parent = commit->parents;

		while (parent) {
			parent->item->object.flags |= SEEN;
			parent = parent->next;
		}

		return 0;
	}


// Source: pack-bitmap.c
// Lines 672-692
