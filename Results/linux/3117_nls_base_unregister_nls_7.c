int unregister_nls(struct nls_table * nls)
{
	struct nls_table ** tmp = &tables;

	spin_lock(&nls_lock);
	while (*tmp) {
		if (nls == *tmp) {
			*tmp = nls->next;
			spin_unlock(&nls_lock);
			return 0;
		}
		tmp = &(*tmp)->next;
	}
	spin_unlock(&nls_lock);
	return -EINVAL;
}


// Source: nls_base.c
// Lines 258-273
