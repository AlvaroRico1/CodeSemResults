static void nfs_pgio_release(void *calldata)
{
	struct nfs_pgio_header *hdr = calldata;
	hdr->completion_ops->completion(hdr);
}


// Source: pagelist.c
// Lines 673-677
