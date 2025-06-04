static int nfs_volume_list_show(struct seq_file *m, void *v)
{
	struct nfs_server *server;
	struct nfs_client *clp;
	char dev[13];	// 8 for 2^24, 1 for ':', 3 for 2^8, 1 for '\0'
	char fsid[34];	// 2 * 16 for %llx, 1 for ':', 1 for '\0'
	struct nfs_net *nn = net_generic(seq_file_net(m), nfs_net_id);

	/* display header on line 1 */
	if (v == &nn->nfs_volume_list) {
		seq_puts(m, "NV SERVER   PORT DEV          FSID"
			    "                              FSC\n");
		return 0;
	}
	/* display one transport per line on subsequent lines */
	server = list_entry(v, struct nfs_server, master_link);
	clp = server->nfs_client;

	snprintf(dev, sizeof(dev), "%u:%u",
		 MAJOR(server->s_dev), MINOR(server->s_dev));

	snprintf(fsid, sizeof(fsid), "%llx:%llx",
		 (unsigned long long) server->fsid.major,
		 (unsigned long long) server->fsid.minor);

	rcu_read_lock();
	seq_printf(m, "v%u %s %s %-12s %-33s %s\n",
		   clp->rpc_ops->version,
		   rpc_peeraddr2str(clp->cl_rpcclient, RPC_DISPLAY_HEX_ADDR),
		   rpc_peeraddr2str(clp->cl_rpcclient, RPC_DISPLAY_HEX_PORT),
		   dev,
		   fsid,
		   nfs_server_fscache_state(server));
	rcu_read_unlock();

	return 0;
}


// Source: client.c
// Lines 1222-1258
