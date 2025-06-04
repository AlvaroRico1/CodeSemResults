static int init_object_disambiguation(struct repository *r,
				      const char *name, int len,
				      struct disambiguate_state *ds)
{
	int i;

	if (len < MINIMUM_ABBREV || len > the_hash_algo->hexsz)
		return -1;

	memset(ds, 0, sizeof(*ds));

	for (i = 0; i < len ;i++) {
		unsigned char c = name[i];
		unsigned char val;
		if (c >= '0' && c <= '9')
			val = c - '0';
		else if (c >= 'a' && c <= 'f')
			val = c - 'a' + 10;
		else if (c >= 'A' && c <='F') {
			val = c - 'A' + 10;
			c -= 'A' - 'a';
		}
		else
			return -1;
		ds->hex_pfx[i] = c;
		if (!(i & 1))
			val <<= 4;
		ds->bin_pfx.hash[i >> 1] |= val;
	}

	ds->len = len;
	ds->hex_pfx[len] = '\0';
	ds->repo = r;
	prepare_alt_odb(r);
	return 0;
}

static int show_ambiguous_object(const struct object_id *oid, void *data)
{
	const struct disambiguate_state *ds = data;
	struct strbuf desc = STRBUF_INIT;
	int type;

	if (ds->fn && !ds->fn(ds->repo, oid, ds->cb_data))
		return 0;

	type = oid_object_info(ds->repo, oid, NULL);
	if (type == OBJ_COMMIT) {
		struct commit *commit = lookup_commit(ds->repo, oid);
		if (commit) {
			struct pretty_print_context pp = {0};
			pp.date_mode.type = DATE_SHORT;
			format_commit_message(commit, " %ad - %s", &desc, &pp);
		}
	} else if (type == OBJ_TAG) {
		struct tag *tag = lookup_tag(ds->repo, oid);
		if (!parse_tag(tag) && tag->tag)
			strbuf_addf(&desc, " %s", tag->tag);
	}

	advise("  %s %s%s",
	       repo_find_unique_abbrev(ds->repo, oid, DEFAULT_ABBREV),
	       type_name(type) ? type_name(type) : "unknown type",
	       desc.buf);

	strbuf_release(&desc);
	return 0;
}

static int collect_ambiguous(const struct object_id *oid, void *data)
{
	oid_array_append(data, oid);
	return 0;
}

static int repo_collect_ambiguous(struct repository *r,
				  const struct object_id *oid,
				  void *data)
{
	return collect_ambiguous(oid, data);
}

static int sort_ambiguous(const void *a, const void *b, void *ctx)
{
	struct repository *sort_ambiguous_repo = ctx;
	int a_type = oid_object_info(sort_ambiguous_repo, a, NULL);
	int b_type = oid_object_info(sort_ambiguous_repo, b, NULL);
	int a_type_sort;
	int b_type_sort;

	/*
	 * Sorts by hash within the same object type, just as
	 * oid_array_for_each_unique() would do.
	 */
	if (a_type == b_type)
		return oidcmp(a, b);

	/*
	 * Between object types show tags, then commits, and finally
	 * trees and blobs.
	 *
	 * The object_type enum is commit, tree, blob, tag, but we
	 * want tag, commit, tree blob. Cleverly (perhaps too
	 * cleverly) do that with modulus, since the enum assigns 1 to
	 * commit, so tag becomes 0.
	 */
	a_type_sort = a_type % 4;
	b_type_sort = b_type % 4;
	return a_type_sort > b_type_sort ? 1 : -1;
}

static void sort_ambiguous_oid_array(struct repository *r, struct oid_array *a)
{
	QSORT_S(a->oid, a->nr, sort_ambiguous, r);
}

static enum get_oid_result get_short_oid(struct repository *r,
					 const char *name, int len,
					 struct object_id *oid,
					 unsigned flags)
{
	int status;
	struct disambiguate_state ds;
	int quietly = !!(flags & GET_OID_QUIETLY);

	if (init_object_disambiguation(r, name, len, &ds) < 0)
		return -1;

	if (HAS_MULTI_BITS(flags & GET_OID_DISAMBIGUATORS))
		BUG("multiple get_short_oid disambiguator flags");

	if (flags & GET_OID_COMMIT)
		ds.fn = disambiguate_commit_only;
	else if (flags & GET_OID_COMMITTISH)
		ds.fn = disambiguate_committish_only;
	else if (flags & GET_OID_TREE)
		ds.fn = disambiguate_tree_only;
	else if (flags & GET_OID_TREEISH)
		ds.fn = disambiguate_treeish_only;
	else if (flags & GET_OID_BLOB)
		ds.fn = disambiguate_blob_only;
	else
		ds.fn = default_disambiguate_hint;

	find_short_object_filename(&ds);
	find_short_packed_object(&ds);
	status = finish_object_disambiguation(&ds, oid);

	/*
	 * If we didn't find it, do the usual reprepare() slow-path,
	 * since the object may have recently been added to the repository
	 * or migrated from loose to packed.
	 */
	if (status == MISSING_OBJECT) {
		reprepare_packed_git(r);
		find_short_object_filename(&ds);
		find_short_packed_object(&ds);
		status = finish_object_disambiguation(&ds, oid);
	}

	if (!quietly && (status == SHORT_NAME_AMBIGUOUS)) {
		struct oid_array collect = OID_ARRAY_INIT;

		error(_("short object ID %s is ambiguous"), ds.hex_pfx);

		/*
		 * We may still have ambiguity if we simply saw a series of
		 * candidates that did not satisfy our hint function. In
		 * that case, we still want to show them, so disable the hint
		 * function entirely.
		 */
		if (!ds.ambiguous)
			ds.fn = NULL;

		advise(_("The candidates are:"));
		repo_for_each_abbrev(r, ds.hex_pfx, collect_ambiguous, &collect);
		sort_ambiguous_oid_array(r, &collect);

		if (oid_array_for_each(&collect, show_ambiguous_object, &ds))
			BUG("show_ambiguous_object shouldn't return non-zero");
		oid_array_clear(&collect);
	}

	return status;
}

int repo_for_each_abbrev(struct repository *r, const char *prefix,
			 each_abbrev_fn fn, void *cb_data)
{
	struct oid_array collect = OID_ARRAY_INIT;
	struct disambiguate_state ds;
	int ret;

	if (init_object_disambiguation(r, prefix, strlen(prefix), &ds) < 0)
		return -1;

	ds.always_call_fn = 1;
	ds.fn = repo_collect_ambiguous;
	ds.cb_data = &collect;
	find_short_object_filename(&ds);
	find_short_packed_object(&ds);

	ret = oid_array_for_each_unique(&collect, fn, cb_data);
	oid_array_clear(&collect);
	return ret;
}

/*
 * Return the slot of the most-significant bit set in "val". There are various
 * ways to do this quickly with fls() or __builtin_clzl(), but speed is
 * probably not a big deal here.
 */
static unsigned msb(unsigned long val)
{
	unsigned r = 0;
	while (val >>= 1)
		r++;
	return r;
}

struct min_abbrev_data {
	unsigned int init_len;
	unsigned int cur_len;
	char *hex;
	struct repository *repo;
	const struct object_id *oid;
};

static inline char get_hex_char_from_oid(const struct object_id *oid,
					 unsigned int pos)
{
	static const char hex[] = "0123456789abcdef";

	if ((pos & 1) == 0)
		return hex[oid->hash[pos >> 1] >> 4];
	else
		return hex[oid->hash[pos >> 1] & 0xf];
}

static int extend_abbrev_len(const struct object_id *oid, void *cb_data)
{
	struct min_abbrev_data *mad = cb_data;

	unsigned int i = mad->init_len;
	while (mad->hex[i] && mad->hex[i] == get_hex_char_from_oid(oid, i))
		i++;

	if (i < GIT_MAX_RAWSZ && i >= mad->cur_len)
		mad->cur_len = i + 1;

	return 0;
}

static int repo_extend_abbrev_len(struct repository *r,
				  const struct object_id *oid,
				  void *cb_data)
{
	return extend_abbrev_len(oid, cb_data);
}

static void find_abbrev_len_for_midx(struct multi_pack_index *m,
				     struct min_abbrev_data *mad)
{
	int match = 0;
	uint32_t num, first = 0;
	struct object_id oid;
	const struct object_id *mad_oid;

	if (!m->num_objects)
		return;

	num = m->num_objects;
	mad_oid = mad->oid;
	match = bsearch_midx(mad_oid, m, &first);

	/*
	 * first is now the position in the packfile where we would insert
	 * mad->hash if it does not exist (or the position of mad->hash if
	 * it does exist). Hence, we consider a maximum of two objects
	 * nearby for the abbreviation length.
	 */
	mad->init_len = 0;
	if (!match) {
		if (nth_midxed_object_oid(&oid, m, first))
			extend_abbrev_len(&oid, mad);
	} else if (first < num - 1) {
		if (nth_midxed_object_oid(&oid, m, first + 1))
			extend_abbrev_len(&oid, mad);
	}
	if (first > 0) {
		if (nth_midxed_object_oid(&oid, m, first - 1))
			extend_abbrev_len(&oid, mad);
	}
	mad->init_len = mad->cur_len;
}

static void find_abbrev_len_for_pack(struct packed_git *p,
				     struct min_abbrev_data *mad)
{
	int match = 0;
	uint32_t num, first = 0;
	struct object_id oid;
	const struct object_id *mad_oid;

	if (p->multi_pack_index)
		return;

	if (open_pack_index(p) || !p->num_objects)
		return;

	num = p->num_objects;
	mad_oid = mad->oid;
	match = bsearch_pack(mad_oid, p, &first);

	/*
	 * first is now the position in the packfile where we would insert
	 * mad->hash if it does not exist (or the position of mad->hash if
	 * it does exist). Hence, we consider a maximum of two objects
	 * nearby for the abbreviation length.
	 */
	mad->init_len = 0;
	if (!match) {
		if (!nth_packed_object_id(&oid, p, first))
			extend_abbrev_len(&oid, mad);
	} else if (first < num - 1) {
		if (!nth_packed_object_id(&oid, p, first + 1))
			extend_abbrev_len(&oid, mad);
	}
	if (first > 0) {
		if (!nth_packed_object_id(&oid, p, first - 1))
			extend_abbrev_len(&oid, mad);
	}
	mad->init_len = mad->cur_len;
}

static void find_abbrev_len_packed(struct min_abbrev_data *mad)
{
	struct multi_pack_index *m;
	struct packed_git *p;

	for (m = get_multi_pack_index(mad->repo); m; m = m->next)
		find_abbrev_len_for_midx(m, mad);
	for (p = get_packed_git(mad->repo); p; p = p->next)
		find_abbrev_len_for_pack(p, mad);
}

int repo_find_unique_abbrev_r(struct repository *r, char *hex,
			      const struct object_id *oid, int len)
{
	struct disambiguate_state ds;
	struct min_abbrev_data mad;
	struct object_id oid_ret;
	const unsigned hexsz = r->hash_algo->hexsz;

	if (len < 0) {
		unsigned long count = repo_approximate_object_count(r);
		/*
		 * Add one because the MSB only tells us the highest bit set,
		 * not including the value of all the _other_ bits (so "15"
		 * is only one off of 2^4, but the MSB is the 3rd bit.
		 */
		len = msb(count) + 1;
		/*
		 * We now know we have on the order of 2^len objects, which
		 * expects a collision at 2^(len/2). But we also care about hex
		 * chars, not bits, and there are 4 bits per hex. So all
		 * together we need to divide by 2 and round up.
		 */
		len = DIV_ROUND_UP(len, 2);
		/*
		 * For very small repos, we stick with our regular fallback.
		 */
		if (len < FALLBACK_DEFAULT_ABBREV)
			len = FALLBACK_DEFAULT_ABBREV;
	}

	oid_to_hex_r(hex, oid);
	if (len == hexsz || !len)
		return hexsz;

	mad.repo = r;
	mad.init_len = len;
	mad.cur_len = len;
	mad.hex = hex;
	mad.oid = oid;

	find_abbrev_len_packed(&mad);

	if (init_object_disambiguation(r, hex, mad.cur_len, &ds) < 0)
		return -1;

	ds.fn = repo_extend_abbrev_len;
	ds.always_call_fn = 1;
	ds.cb_data = (void *)&mad;

	find_short_object_filename(&ds);
	(void)finish_object_disambiguation(&ds, &oid_ret);

	hex[mad.cur_len] = 0;
	return mad.cur_len;
}

const char *repo_find_unique_abbrev(struct repository *r,
				    const struct object_id *oid,
				    int len)
{
	static int bufno;
	static char hexbuffer[4][GIT_MAX_HEXSZ + 1];
	char *hex = hexbuffer[bufno];
	bufno = (bufno + 1) % ARRAY_SIZE(hexbuffer);
	repo_find_unique_abbrev_r(r, hex, oid, len);
	return hex;
}

static int ambiguous_path(const char *path, int len)
{
	int slash = 1;
	int cnt;

	for (cnt = 0; cnt < len; cnt++) {
		switch (*path++) {
		case '\0':
			break;
		case '/':
			if (slash)
				break;
			slash = 1;
			continue;
		case '.':
			continue;
		default:
			slash = 0;
			continue;
		}
		break;
	}
	return slash;
}

static inline int at_mark(const char *string, int len,
			  const char **suffix, int nr)
{
	int i;

	for (i = 0; i < nr; i++) {
		int suffix_len = strlen(suffix[i]);
		if (suffix_len <= len
		    && !strncasecmp(string, suffix[i], suffix_len))
			return suffix_len;
	}
	return 0;
}

static inline int upstream_mark(const char *string, int len)
{
	const char *suffix[] = { "@{upstream}", "@{u}" };
	return at_mark(string, len, suffix, ARRAY_SIZE(suffix));
}

static inline int push_mark(const char *string, int len)
{
	const char *suffix[] = { "@{push}" };
	return at_mark(string, len, suffix, ARRAY_SIZE(suffix));
}

static enum get_oid_result get_oid_1(struct repository *r, const char *name, int len, struct object_id *oid, unsigned lookup_flags);
static int interpret_nth_prior_checkout(struct repository *r, const char *name, int namelen, struct strbuf *buf);

static int get_oid_basic(struct repository *r, const char *str, int len,
			 struct object_id *oid, unsigned int flags)
{
	static const char *warn_msg = "refname '%.*s' is ambiguous.";
	static const char *object_name_msg = N_(
	"Git normally never creates a ref that ends with 40 hex characters\n"
	"because it will be ignored when you just specify 40-hex. These refs\n"
	"may be created by mistake. For example,\n"
	"\n"
	"  git switch -c $br $(git rev-parse ...)\n"
	"\n"
	"where \"$br\" is somehow empty and a 40-hex ref is created. Please\n"
	"examine these refs and maybe delete them. Turn this message off by\n"
	"running \"git config advice.objectNameWarning false\"");
	struct object_id tmp_oid;
	char *real_ref = NULL;
	int refs_found = 0;
	int at, reflog_len, nth_prior = 0;

	if (len == r->hash_algo->hexsz && !get_oid_hex(str, oid)) {
		if (warn_ambiguous_refs && warn_on_object_refname_ambiguity) {
			refs_found = repo_dwim_ref(r, str, len, &tmp_oid, &real_ref, 0);
			if (refs_found > 0) {
				warning(warn_msg, len, str);
				if (advice_enabled(ADVICE_OBJECT_NAME_WARNING))
					fprintf(stderr, "%s\n", _(object_name_msg));
			}
			free(real_ref);
		}
		return 0;
	}

	/* basic@{time or number or -number} format to query ref-log */
	reflog_len = at = 0;
	if (len && str[len-1] == '}') {
		for (at = len-4; at >= 0; at--) {
			if (str[at] == '@' && str[at+1] == '{') {
				if (str[at+2] == '-') {
					if (at != 0)
						/* @{-N} not at start */
						return -1;
					nth_prior = 1;
					continue;
				}
				if (!upstream_mark(str + at, len - at) &&
				    !push_mark(str + at, len - at)) {
					reflog_len = (len-1) - (at+2);
					len = at;
				}
				break;
			}
		}
	}

	/* Accept only unambiguous ref paths. */
	if (len && ambiguous_path(str, len))
		return -1;

	if (nth_prior) {
		struct strbuf buf = STRBUF_INIT;
		int detached;

		if (interpret_nth_prior_checkout(r, str, len, &buf) > 0) {
			detached = (buf.len == r->hash_algo->hexsz && !get_oid_hex(buf.buf, oid));
			strbuf_release(&buf);
			if (detached)
				return 0;
		}
	}

	if (!len && reflog_len)
		/* allow "@{...}" to mean the current branch reflog */
		refs_found = repo_dwim_ref(r, "HEAD", 4, oid, &real_ref, 0);
	else if (reflog_len)
		refs_found = repo_dwim_log(r, str, len, oid, &real_ref);
	else
		refs_found = repo_dwim_ref(r, str, len, oid, &real_ref, 0);

	if (!refs_found)
		return -1;

	if (warn_ambiguous_refs && !(flags & GET_OID_QUIETLY) &&
	    (refs_found > 1 ||
	     !get_short_oid(r, str, len, &tmp_oid, GET_OID_QUIETLY)))
		warning(warn_msg, len, str);

	if (reflog_len) {
		int nth, i;
		timestamp_t at_time;
		timestamp_t co_time;
		int co_tz, co_cnt;

		/* Is it asking for N-th entry, or approxidate? */
		for (i = nth = 0; 0 <= nth && i < reflog_len; i++) {
			char ch = str[at+2+i];
			if ('0' <= ch && ch <= '9')
				nth = nth * 10 + ch - '0';
			else
				nth = -1;
		}
		if (100000000 <= nth) {
			at_time = nth;
			nth = -1;
		} else if (0 <= nth)
			at_time = 0;
		else {
			int errors = 0;
			char *tmp = xstrndup(str + at + 2, reflog_len);
			at_time = approxidate_careful(tmp, &errors);
			free(tmp);
			if (errors) {
				free(real_ref);
				return -1;
			}
		}
		if (read_ref_at(get_main_ref_store(r),
				real_ref, flags, at_time, nth, oid, NULL,
				&co_time, &co_tz, &co_cnt)) {
			if (!len) {
				if (!skip_prefix(real_ref, "refs/heads/", &str))
					str = "HEAD";
				len = strlen(str);
			}
			if (at_time) {
				if (!(flags & GET_OID_QUIETLY)) {
					warning(_("log for '%.*s' only goes back to %s"),
						len, str,
						show_date(co_time, co_tz, DATE_MODE(RFC2822)));
				}
			} else {
				if (flags & GET_OID_QUIETLY) {
					exit(128);
				}
				die(_("log for '%.*s' only has %d entries"),
				    len, str, co_cnt);
			}
		}
	}

	free(real_ref);
	return 0;
}

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

struct object *repo_peel_to_type(struct repository *r, const char *name, int namelen,
				 struct object *o, enum object_type expected_type)
{
	if (name && !namelen)
		namelen = strlen(name);
	while (1) {
		if (!o || (!o->parsed && !parse_object(r, &o->oid)))
			return NULL;
		if (expected_type == OBJ_ANY || o->type == expected_type)
			return o;
		if (o->type == OBJ_TAG)
			o = ((struct tag*) o)->tagged;
		else if (o->type == OBJ_COMMIT)
			o = &(repo_get_commit_tree(r, ((struct commit *)o))->object);
		else {
			if (name)
				error("%.*s: expected %s type, but the object "
				      "dereferences to %s type",
				      namelen, name, type_name(expected_type),
				      type_name(o->type));
			return NULL;
		}
	}
}

static int peel_onion(struct repository *r, const char *name, int len,
		      struct object_id *oid, unsigned lookup_flags)
{
	struct object_id outer;
	const char *sp;
	unsigned int expected_type = 0;
	struct object *o;

	/*
	 * "ref^{type}" dereferences ref repeatedly until you cannot
	 * dereference anymore, or you get an object of given type,
	 * whichever comes first.  "ref^{}" means just dereference
	 * tags until you get a non-tag.  "ref^0" is a shorthand for
	 * "ref^{commit}".  "commit^{tree}" could be used to find the
	 * top-level tree of the given commit.
	 */
	if (len < 4 || name[len-1] != '}')
		return -1;

	for (sp = name + len - 1; name <= sp; sp--) {
		int ch = *sp;
		if (ch == '{' && name < sp && sp[-1] == '^')
			break;
	}
	if (sp <= name)
		return -1;

	sp++; /* beginning of type name, or closing brace for empty */
	if (starts_with(sp, "commit}"))
		expected_type = OBJ_COMMIT;
	else if (starts_with(sp, "tag}"))


// Source: object-name.c
// Lines 317-1038
