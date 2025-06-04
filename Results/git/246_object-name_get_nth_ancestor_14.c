static enum get_oid_result get_nth_ancestor(struct repository *r,
					    const char *name, int len,
					    struct object_id *result,
					    int generation)
{
	struct object_id oid;
	struct commit *commit;
	int ret;

	ret = get_oid_1(r, name, len, &oid, GET_OID_COMMITTISH);
	if (ret)
		return ret;
	commit = lookup_commit_reference(r, &oid);
	if (!commit)
		return MISSING_OBJECT;

	while (generation--) {
		if (parse_commit(commit) || !commit->parents)
			return MISSING_OBJECT;
		commit = commit->parents->item;
	}
	oidcpy(result, &commit->object.oid);
	return FOUND;
}


// Source: object-name.c
// Lines 958-981
