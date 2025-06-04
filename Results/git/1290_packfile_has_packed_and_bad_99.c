const struct packed_git *has_packed_and_bad(struct repository *r,
					    const struct object_id *oid)
{
	struct packed_git *p;

	for (p = r->objects->packed_git; p; p = p->next)
		if (oidset_contains(&p->bad_objects, oid))
			return p;
	return NULL;
}


// Source: packfile.c
// Lines 1170-1179
