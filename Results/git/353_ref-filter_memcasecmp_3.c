static int memcasecmp(const void *vs1, const void *vs2, size_t n)
{
	const char *s1 = vs1, *s2 = vs2;
	const char *end = s1 + n;

	for (; s1 < end; s1++, s2++) {
		int diff = tolower(*s1) - tolower(*s2);
		if (diff)
			return diff;
	}
	return 0;
}

static int cmp_ref_sorting(struct ref_sorting *s, struct ref_array_item *a, struct ref_array_item *b)
{
	struct atom_value *va, *vb;
	int cmp;
	int cmp_detached_head = 0;
	cmp_type cmp_type = used_atom[s->atom].type;
	struct strbuf err = STRBUF_INIT;

	if (get_ref_atom_value(a, s->atom, &va, &err))
		die("%s", err.buf);
	if (get_ref_atom_value(b, s->atom, &vb, &err))
		die("%s", err.buf);
	strbuf_release(&err);
	if (s->sort_flags & REF_SORTING_DETACHED_HEAD_FIRST &&
	    ((a->kind | b->kind) & FILTER_REFS_DETACHED_HEAD)) {
		cmp = compare_detached_head(a, b);
		cmp_detached_head = 1;
	} else if (s->sort_flags & REF_SORTING_VERSION) {
		cmp = versioncmp(va->s, vb->s);
	} else if (cmp_type == FIELD_STR) {
		if (va->s_size < 0 && vb->s_size < 0) {
			int (*cmp_fn)(const char *, const char *);
			cmp_fn = s->sort_flags & REF_SORTING_ICASE
				? strcasecmp : strcmp;
			cmp = cmp_fn(va->s, vb->s);
		} else {
			size_t a_size = va->s_size < 0 ?
					strlen(va->s) : va->s_size;
			size_t b_size = vb->s_size < 0 ?
					strlen(vb->s) : vb->s_size;
			int (*cmp_fn)(const void *, const void *, size_t);
			cmp_fn = s->sort_flags & REF_SORTING_ICASE
				? memcasecmp : memcmp;

			cmp = cmp_fn(va->s, vb->s, b_size > a_size ?
				     a_size : b_size);
			if (!cmp) {
				if (a_size > b_size)
					cmp = 1;
				else if (a_size < b_size)
					cmp = -1;
			}
		}
	} else {
		if (va->value < vb->value)
			cmp = -1;
		else if (va->value == vb->value)
			cmp = 0;
		else
			cmp = 1;
	}

	return (s->sort_flags & REF_SORTING_REVERSE && !cmp_detached_head)
		? -cmp : cmp;
}


// Source: ref-filter.c
// Lines 2460-2527
