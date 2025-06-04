static int match_explicit(struct ref *src, struct ref *dst,
			  struct ref ***dst_tail,
			  struct refspec_item *rs)
{
	struct ref *matched_src, *matched_dst;
	int allocated_src;

	const char *dst_value = rs->dst;
	char *dst_guess;

	if (rs->pattern || rs->matching || rs->negative)
		return 0;

	matched_src = matched_dst = NULL;
	if (match_explicit_lhs(src, rs, &matched_src, &allocated_src) < 0)
		return -1;

	if (!dst_value) {
		int flag;

		dst_value = resolve_ref_unsafe(matched_src->name,
					       RESOLVE_REF_READING,
					       NULL, &flag);
		if (!dst_value ||
		    ((flag & REF_ISSYMREF) &&
		     !starts_with(dst_value, "refs/heads/")))
			die(_("%s cannot be resolved to branch"),
			    matched_src->name);
	}


// Source: remote.c
// Lines 1148-1176
