static void nfs_show_mountd_options(struct seq_file *m, struct nfs_server *nfss,
				    int showdefaults)
{
	struct sockaddr *sap = (struct sockaddr *)&nfss->mountd_address;

	if (nfss->flags & NFS_MOUNT_LEGACY_INTERFACE)
		return;

	switch (sap->sa_family) {
	case AF_INET: {
		struct sockaddr_in *sin = (struct sockaddr_in *)sap;
		seq_printf(m, ",mountaddr=%pI4", &sin->sin_addr.s_addr);
		break;
	}


// Source: super.c
// Lines 567-580
