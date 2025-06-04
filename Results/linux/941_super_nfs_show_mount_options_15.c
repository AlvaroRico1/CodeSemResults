static void nfs_show_mount_options(struct seq_file *m, struct nfs_server *nfss,
				   int showdefaults)
{
	static const struct proc_nfs_info {
		int flag;
		const char *str;
		const char *nostr;
	} nfs_info[] = {
		{ NFS_MOUNT_SOFT, ",soft", "" },
		{ NFS_MOUNT_SOFTERR, ",softerr", "" },
		{ NFS_MOUNT_POSIX, ",posix", "" },
		{ NFS_MOUNT_NOCTO, ",nocto", "" },
		{ NFS_MOUNT_NOAC, ",noac", "" },
		{ NFS_MOUNT_NONLM, ",nolock", "" },
		{ NFS_MOUNT_NOACL, ",noacl", "" },
		{ NFS_MOUNT_NORDIRPLUS, ",nordirplus", "" },
		{ NFS_MOUNT_UNSHARED, ",nosharecache", "" },
		{ NFS_MOUNT_NORESVPORT, ",noresvport", "" },
		{ 0, NULL, NULL }
	};
	const struct proc_nfs_info *nfs_infop;
	struct nfs_client *clp = nfss->nfs_client;
	u32 version = clp->rpc_ops->version;
	int local_flock, local_fcntl;

	nfs_show_nfs_version(m, version, clp->cl_minorversion);
	seq_printf(m, ",rsize=%u", nfss->rsize);
	seq_printf(m, ",wsize=%u", nfss->wsize);
	if (nfss->bsize != 0)
		seq_printf(m, ",bsize=%u", nfss->bsize);
	seq_printf(m, ",namlen=%u", nfss->namelen);
	if (nfss->acregmin != NFS_DEF_ACREGMIN*HZ || showdefaults)
		seq_printf(m, ",acregmin=%u", nfss->acregmin/HZ);
	if (nfss->acregmax != NFS_DEF_ACREGMAX*HZ || showdefaults)
		seq_printf(m, ",acregmax=%u", nfss->acregmax/HZ);
	if (nfss->acdirmin != NFS_DEF_ACDIRMIN*HZ || showdefaults)
		seq_printf(m, ",acdirmin=%u", nfss->acdirmin/HZ);
	if (nfss->acdirmax != NFS_DEF_ACDIRMAX*HZ || showdefaults)
		seq_printf(m, ",acdirmax=%u", nfss->acdirmax/HZ);
	if (!(nfss->flags & (NFS_MOUNT_SOFT|NFS_MOUNT_SOFTERR)))
			seq_puts(m, ",hard");
	for (nfs_infop = nfs_info; nfs_infop->flag; nfs_infop++) {
		if (nfss->flags & nfs_infop->flag)
			seq_puts(m, nfs_infop->str);
		else
			seq_puts(m, nfs_infop->nostr);
	}
	rcu_read_lock();
	seq_printf(m, ",proto=%s",
		   rpc_peeraddr2str(nfss->client, RPC_DISPLAY_NETID));
	rcu_read_unlock();
	if (clp->cl_nconnect > 0)
		seq_printf(m, ",nconnect=%u", clp->cl_nconnect);
	if (version == 4) {
		if (nfss->port != NFS_PORT)
			seq_printf(m, ",port=%u", nfss->port);
	} else
		if (nfss->port)
			seq_printf(m, ",port=%u", nfss->port);

	seq_printf(m, ",timeo=%lu", 10U * nfss->client->cl_timeout->to_initval / HZ);
	seq_printf(m, ",retrans=%u", nfss->client->cl_timeout->to_retries);
	seq_printf(m, ",sec=%s", nfs_pseudoflavour_to_name(nfss->client->cl_auth->au_flavor));

	if (version != 4)
		nfs_show_mountd_options(m, nfss, showdefaults);
	else
		nfs_show_nfsv4_options(m, nfss, showdefaults);

	if (nfss->options & NFS_OPTION_FSCACHE)
		seq_puts(m, ",fsc");

	if (nfss->options & NFS_OPTION_MIGRATION)
		seq_puts(m, ",migration");

	if (nfss->flags & NFS_MOUNT_LOOKUP_CACHE_NONEG) {
		if (nfss->flags & NFS_MOUNT_LOOKUP_CACHE_NONE)
			seq_puts(m, ",lookupcache=none");
		else
			seq_puts(m, ",lookupcache=pos");
	}

	local_flock = nfss->flags & NFS_MOUNT_LOCAL_FLOCK;
	local_fcntl = nfss->flags & NFS_MOUNT_LOCAL_FCNTL;

	if (!local_flock && !local_fcntl)
		seq_puts(m, ",local_lock=none");
	else if (local_flock && local_fcntl)
		seq_puts(m, ",local_lock=all");
	else if (local_flock)
		seq_puts(m, ",local_lock=flock");
	else
		seq_puts(m, ",local_lock=posix");
}


// Source: super.c
// Lines 628-721
