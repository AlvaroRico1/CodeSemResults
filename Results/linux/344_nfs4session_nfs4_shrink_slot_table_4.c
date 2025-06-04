static void nfs4_shrink_slot_table(struct nfs4_slot_table  *tbl, u32 newsize)
{
	struct nfs4_slot **p;
	if (newsize >= tbl->max_slots)
		return;

	p = &tbl->slots;
	while (newsize--)
		p = &(*p)->next;
	while (*p) {
		struct nfs4_slot *slot = *p;

		*p = slot->next;
		kfree(slot);
		tbl->max_slots--;
	}


// Source: nfs4session.c
// Lines 39-54
