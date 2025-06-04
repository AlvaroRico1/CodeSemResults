static void *read_object(struct repository *r,
			 const struct object_id *oid,
			 enum object_type *type,
			 unsigned long *size)
{
	struct object_info oi = OBJECT_INFO_INIT;
	void *content;
	oi.typep = type;
	oi.sizep = size;
	oi.contentp = &content;

	if (oid_object_info_extended(r, oid, &oi, 0) < 0)
		return NULL;
	return content;
}


// Source: packfile.c
// Lines 1649-1663
