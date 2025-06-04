nfs_pageio_alloc_mirrors(struct nfs_pageio_descriptor *desc,
		unsigned int mirror_count)
{
	struct nfs_pgio_mirror *ret;
	unsigned int i;

	kfree(desc->pg_mirrors_dynamic);
	desc->pg_mirrors_dynamic = NULL;
	if (mirror_count == 1)
		return desc->pg_mirrors_static;
	ret = kmalloc_array(mirror_count, sizeof(*ret), GFP_KERNEL);
	if (ret != NULL) {
		for (i = 0; i < mirror_count; i++)
			nfs_pageio_mirror_init(&ret[i], desc->pg_bsize);
		desc->pg_mirrors_dynamic = ret;
	}
	return ret;
}


// Source: pagelist.c
// Lines 842-859
