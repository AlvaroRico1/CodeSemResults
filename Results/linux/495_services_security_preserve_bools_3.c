static int security_preserve_bools(struct selinux_state *state,
				   struct policydb *policydb)
{
	int rc, nbools = 0, *bvalues = NULL, i;
	char **bnames = NULL;
	struct cond_bool_datum *booldatum;
	struct cond_node *cur;

	rc = security_get_bools(state, &nbools, &bnames, &bvalues);
	if (rc)
		goto out;
	for (i = 0; i < nbools; i++) {
		booldatum = hashtab_search(policydb->p_bools.table, bnames[i]);
		if (booldatum)
			booldatum->state = bvalues[i];
	}
	for (cur = policydb->cond_list; cur; cur = cur->next) {
		rc = evaluate_cond_node(policydb, cur);
		if (rc)
			goto out;
	}

out:
	if (bnames) {
		for (i = 0; i < nbools; i++)
			kfree(bnames[i]);
	}
	kfree(bnames);
	kfree(bvalues);
	return rc;
}


// Source: services.c
// Lines 2920-2950
