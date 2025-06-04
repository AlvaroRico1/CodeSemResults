struct nfs_pgio_header *nfs_pgio_header_alloc(const struct nfs_rw_ops *ops)
{
	struct nfs_pgio_header *hdr = ops->rw_alloc_header();

	if (hdr) {
		INIT_LIST_HEAD(&hdr->pages);
		hdr->rw_ops = ops;
	}
	return hdr;
}


// Source: pagelist.c
// Lines 516-525
