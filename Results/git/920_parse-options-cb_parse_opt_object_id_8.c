int parse_opt_object_id(const struct option *opt, const char *arg, int unset)
{
	struct object_id oid;
	struct object_id *target = opt->value;

	if (unset) {
		oidcpy(target, null_oid());
		return 0;
	}
	if (!arg)
		return -1;
	if (get_oid(arg, &oid))
		return error(_("malformed object name '%s'"), arg);
	*target = oid;
	return 0;
}


// Source: parse-options-cb.c
// Lines 137-152
