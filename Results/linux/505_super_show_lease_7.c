static void show_lease(struct seq_file *m, struct nfs_server *server)
{
	struct nfs_client *clp = server->nfs_client;
	unsigned long expire;

	seq_printf(m, ",lease_time=%ld", clp->cl_lease_time / HZ);
	expire = clp->cl_last_renewal + clp->cl_lease_time;
	seq_printf(m, ",lease_expired=%ld",
		   time_after(expire, jiffies) ?  0 : (jiffies - expire) / HZ);
}


// Source: super.c
// Lines 743-752
