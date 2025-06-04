static enum list_objects_filter_result filter_blobs_limit(
	struct repository *r,
	enum list_objects_filter_situation filter_situation,
	struct object *obj,
	const char *pathname,
	const char *filename,
	struct oidset *omits,
	void *filter_data_)
{
	struct filter_blobs_limit_data *filter_data = filter_data_;
	unsigned long object_length;
	enum object_type t;

	switch (filter_situation) {
	default:
		BUG("unknown filter_situation: %d", filter_situation);

	case LOFS_TAG:
		assert(obj->type == OBJ_TAG);
		/* always include all tag objects */
		return LOFR_MARK_SEEN | LOFR_DO_SHOW;

	case LOFS_COMMIT:
		assert(obj->type == OBJ_COMMIT);
		/* always include all commit objects */
		return LOFR_MARK_SEEN | LOFR_DO_SHOW;

	case LOFS_BEGIN_TREE:
		assert(obj->type == OBJ_TREE);
		/* always include all tree objects */
		return LOFR_MARK_SEEN | LOFR_DO_SHOW;

	case LOFS_END_TREE:
		assert(obj->type == OBJ_TREE);
		return LOFR_ZERO;

	case LOFS_BLOB:
		assert(obj->type == OBJ_BLOB);
		assert((obj->flags & SEEN) == 0);

		t = oid_object_info(r, &obj->oid, &object_length);
		if (t != OBJ_BLOB) { /* probably OBJ_NONE */
			/*
			 * We DO NOT have the blob locally, so we cannot
			 * apply the size filter criteria.  Be conservative
			 * and force show it (and let the caller deal with
			 * the ambiguity).
			 */
			goto include_it;
		}

		if (object_length < filter_data->max_bytes)
			goto include_it;

		if (omits)
			oidset_insert(omits, &obj->oid);
		return LOFR_MARK_SEEN; /* but not LOFR_DO_SHOW (hard omit) */
	}

include_it:
	if (omits)
		oidset_remove(omits, &obj->oid);
	return LOFR_MARK_SEEN | LOFR_DO_SHOW;
}


// Source: list-objects-filter.c
// Lines 273-336
