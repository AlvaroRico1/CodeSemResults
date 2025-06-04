static void find_short_object_filename(struct disambiguate_state *ds)
{
	struct object_directory *odb;

	for (odb = ds->repo->objects->odb; odb && !ds->ambiguous; odb = odb->next)
		oidtree_each(odb_loose_cache(odb, &ds->bin_pfx),
				&ds->bin_pfx, ds->len, match_prefix, ds);
}


// Source: object-name.c
// Lines 98-105
