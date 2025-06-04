static enum list_objects_filter_result filter_object_type(
	struct repository *r,
	enum list_objects_filter_situation filter_situation,
	struct object *obj,
	const char *pathname,
	const char *filename,
	struct oidset *omits,
	void *filter_data_)
{
	struct filter_object_type_data *filter_data = filter_data_;

	switch (filter_situation) {
	default:
		BUG("unknown filter_situation: %d", filter_situation);

	case LOFS_TAG:
		assert(obj->type == OBJ_TAG);
		if (filter_data->object_type == OBJ_TAG)
			return LOFR_MARK_SEEN | LOFR_DO_SHOW;
		return LOFR_MARK_SEEN;

	case LOFS_COMMIT:
		assert(obj->type == OBJ_COMMIT);
		if (filter_data->object_type == OBJ_COMMIT)
			return LOFR_MARK_SEEN | LOFR_DO_SHOW;
		return LOFR_MARK_SEEN;

	case LOFS_BEGIN_TREE:
		assert(obj->type == OBJ_TREE);

		/*
		 * If we only want to show commits or tags, then there is no
		 * need to walk down trees.
		 */
		if (filter_data->object_type == OBJ_COMMIT ||
		    filter_data->object_type == OBJ_TAG)
			return LOFR_SKIP_TREE;

		if (filter_data->object_type == OBJ_TREE)
			return LOFR_MARK_SEEN | LOFR_DO_SHOW;

		return LOFR_MARK_SEEN;

	case LOFS_BLOB:
		assert(obj->type == OBJ_BLOB);

		if (filter_data->object_type == OBJ_BLOB)
			return LOFR_MARK_SEEN | LOFR_DO_SHOW;
		return LOFR_MARK_SEEN;

	case LOFS_END_TREE:
		return LOFR_ZERO;
	}
}


// Source: list-objects-filter.c
// Lines 556-609
