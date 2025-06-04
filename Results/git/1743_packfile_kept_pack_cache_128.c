static struct packed_git **kept_pack_cache(struct repository *r, unsigned flags)
{
	maybe_invalidate_kept_pack_cache(r, flags);

	if (!r->objects->kept_pack_cache.packs) {
		struct packed_git **packs = NULL;
		size_t nr = 0, alloc = 0;
		struct packed_git *p;

		/*
		 * We want "all" packs here, because we need to cover ones that
		 * are used by a midx, as well. We need to look in every one of
		 * them (instead of the midx itself) to cover duplicates. It's
		 * possible that an object is found in two packs that the midx
		 * covers, one kept and one not kept, but the midx returns only
		 * the non-kept version.
		 */
		for (p = get_all_packs(r); p; p = p->next) {
			if ((p->pack_keep && (flags & ON_DISK_KEEP_PACKS)) ||
			    (p->pack_keep_in_core && (flags & IN_CORE_KEEP_PACKS))) {
				ALLOC_GROW(packs, nr + 1, alloc);
				packs[nr++] = p;
			}
		}
		ALLOC_GROW(packs, nr + 1, alloc);
		packs[nr] = NULL;

		r->objects->kept_pack_cache.packs = packs;
		r->objects->kept_pack_cache.flags = flags;
	}


// Source: packfile.c
// Lines 2065-2094
