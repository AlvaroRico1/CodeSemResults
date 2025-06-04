static struct nfs4_slot *nfs4_new_slot(struct nfs4_slot_table  *tbl,
		u32 slotid, u32 seq_init, gfp_t gfp_mask)
{
	struct nfs4_slot *slot;

	slot = kzalloc(sizeof(*slot), gfp_mask);
	if (slot) {
		slot->table = tbl;
		slot->slot_nr = slotid;
		slot->seq_nr = seq_init;
		slot->seq_nr_highest_sent = seq_init;
		slot->seq_nr_last_acked = seq_init - 1;
	}
	return slot;
}


// Source: nfs4session.c
// Lines 104-118
