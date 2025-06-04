static void send_info(struct repository *r, struct packet_writer *writer,
		      struct string_list *oid_str_list,
		      struct requested_info *info)
{
	struct string_list_item *item;
	struct strbuf send_buffer = STRBUF_INIT;

	if (!oid_str_list->nr)
		return;

	if (info->size)
		packet_writer_write(writer, "size");

	for_each_string_list_item (item, oid_str_list) {
		const char *oid_str = item->string;
		struct object_id oid;
		unsigned long object_size;

		if (get_oid_hex(oid_str, &oid) < 0) {
			packet_writer_error(
				writer,
				"object-info: protocol error, expected to get oid, not '%s'",
				oid_str);
			continue;
		}

		strbuf_addstr(&send_buffer, oid_str);

		if (info->size) {
			if (oid_object_info(r, &oid, &object_size) < 0) {
				strbuf_addstr(&send_buffer, " ");
			} else {
				strbuf_addf(&send_buffer, " %lu", object_size);
			}
		}

		packet_writer_write(writer, "%s", send_buffer.buf);
		strbuf_reset(&send_buffer);
	}


// Source: protocol-caps.c
// Lines 36-74
