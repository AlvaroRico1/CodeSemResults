static enum rewrite_result line_log_rewrite_one(struct rev_info *rev, struct commit **pp)
{
	for (;;) {
		struct commit *p = *pp;
		if (p->parents && p->parents->next)
			return rewrite_one_ok;
		if (p->object.flags & UNINTERESTING)
			return rewrite_one_ok;
		if (!(p->object.flags & TREESAME))
			return rewrite_one_ok;
		if (!p->parents)
			return rewrite_one_noparents;
		*pp = p->parents->item;
	}


// Source: line-log.c
// Lines 1285-1298
