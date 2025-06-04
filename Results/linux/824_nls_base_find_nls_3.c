static struct nls_table *find_nls(char *charset)
{
	struct nls_table *nls;
	spin_lock(&nls_lock);
	for (nls = tables; nls; nls = nls->next) {
		if (!strcmp(nls->charset, charset))
			break;
		if (nls->alias && !strcmp(nls->alias, charset))
			break;
	}
	if (nls && !try_module_get(nls->owner))
		nls = NULL;
	spin_unlock(&nls_lock);
	return nls;
}


// Source: nls_base.c
// Lines 275-289
