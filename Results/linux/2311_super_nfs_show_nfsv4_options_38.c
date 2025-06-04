static void nfs_show_nfsv4_options(struct seq_file *m, struct nfs_server *nfss,
				    int showdefaults)
{
	struct nfs_client *clp = nfss->nfs_client;

	seq_printf(m, ",clientaddr=%s", clp->cl_ipaddr);
}


// Source: super.c
// Lines 602-608
