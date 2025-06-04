int nfs_writepages(struct address_space *mapping, struct writeback_control *wbc)
{
	struct inode *inode = mapping->host;
	struct nfs_pageio_descriptor pgio;
	struct nfs_io_completion *ioc;
	int err;

	nfs_inc_stats(inode, NFSIOS_VFSWRITEPAGES);

	ioc = nfs_io_completion_alloc(GFP_KERNEL);
	if (ioc)
		nfs_io_completion_init(ioc, nfs_io_completion_commit, inode);

	nfs_pageio_init_write(&pgio, inode, wb_priority(wbc), false,
				&nfs_async_write_completion_ops);
	pgio.pg_io_completion = ioc;
	err = write_cache_pages(mapping, wbc, nfs_writepages_callback, &pgio);
	pgio.pg_error = 0;
	nfs_pageio_complete(&pgio);
	nfs_io_completion_put(ioc);

	if (err < 0)
		goto out_err;
	err = pgio.pg_error;
	if (nfs_error_is_fatal(err))
		goto out_err;
	return 0;
out_err:
	return err;
}


// Source: write.c
// Lines 712-741
