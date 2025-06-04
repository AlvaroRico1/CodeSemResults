static void send_one_item(int fd, struct parallel_checkout_item *pc_item)
{
	size_t len_data;
	char *data, *variant;
	struct pc_item_fixed_portion *fixed_portion;
	const char *working_tree_encoding = pc_item->ca.working_tree_encoding;
	size_t name_len = pc_item->ce->ce_namelen;
	size_t working_tree_encoding_len = working_tree_encoding ?
					   strlen(working_tree_encoding) : 0;

	/*
	 * Any changes in the calculation of the message size must also be made
	 * in is_eligible_for_parallel_checkout().
	 */
	len_data = sizeof(struct pc_item_fixed_portion) + name_len +
		   working_tree_encoding_len;

	data = xmalloc(len_data);

	fixed_portion = (struct pc_item_fixed_portion *)data;
	fixed_portion->id = pc_item->id;
	fixed_portion->ce_mode = pc_item->ce->ce_mode;
	fixed_portion->crlf_action = pc_item->ca.crlf_action;
	fixed_portion->ident = pc_item->ca.ident;
	fixed_portion->name_len = name_len;
	fixed_portion->working_tree_encoding_len = working_tree_encoding_len;
	/*
	 * We pad the unused bytes in the hash array because, otherwise,
	 * Valgrind would complain about passing uninitialized bytes to a
	 * write() syscall. The warning doesn't represent any real risk here,
	 * but it could hinder the detection of actual errors.
	 */
	oidcpy_with_padding(&fixed_portion->oid, &pc_item->ce->oid);

	variant = data + sizeof(*fixed_portion);
	if (working_tree_encoding_len) {
		memcpy(variant, working_tree_encoding, working_tree_encoding_len);
		variant += working_tree_encoding_len;
	}
	memcpy(variant, pc_item->ce->name, name_len);

	packet_write(fd, data, len_data);

	free(data);
}


// Source: parallel-checkout.c
// Lines 397-441
