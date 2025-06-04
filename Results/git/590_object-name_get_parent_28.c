static enum get_oid_result get_parent(struct repository *r,
				      const char *name, int len,
				      struct object_id *result, int idx)
{
	struct object_id oid;
	enum get_oid_result ret = get_oid_1(r, name, len, &oid,
					    GET_OID_COMMITTISH);
	struct commit *commit;
	struct commit_list *p;

	if (ret)
		return ret;
	commit = lookup_commit_reference(r, &oid);
	if (parse_commit(commit))
		return MISSING_OBJECT;
	if (!idx) {
		oidcpy(result, &commit->object.oid);
		return FOUND;
	}
	p = commit->parents;
	while (p) {
		if (!--idx) {
			oidcpy(result, &p->item->object.oid);
			return FOUND;
		}
		p = p->next;
	}
	return MISSING_OBJECT;
}


// Source: object-name.c
// Lines 928-956
