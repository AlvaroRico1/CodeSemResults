static void clear_common_flag(struct oidset *s)
{
	struct oidset_iter iter;
	const struct object_id *oid;
	oidset_iter_init(s, &iter);

	while ((oid = oidset_iter_next(&iter))) {
		struct object *obj = lookup_object(the_repository, oid);
		obj->flags &= ~COMMON;
	}
}

void negotiate_using_fetch(const struct oid_array *negotiation_tips,
			   const struct string_list *server_options,
			   int stateless_rpc,
			   int fd[],
			   struct oidset *acked_commits)
{
	struct fetch_negotiator negotiator;
	struct packet_reader reader;
	struct object_array nt_object_array = OBJECT_ARRAY_INIT;
	struct strbuf req_buf = STRBUF_INIT;
	int haves_to_send = INITIAL_FLUSH;
	int in_vain = 0;
	int seen_ack = 0;
	int last_iteration = 0;
	timestamp_t min_generation = GENERATION_NUMBER_INFINITY;

	fetch_negotiator_init(the_repository, &negotiator);
	mark_tips(&negotiator, negotiation_tips);

	packet_reader_init(&reader, fd[0], NULL, 0,
			   PACKET_READ_CHOMP_NEWLINE |
			   PACKET_READ_DIE_ON_ERR_PACKET);

	oid_array_for_each((struct oid_array *) negotiation_tips,
			   add_to_object_array,
			   &nt_object_array);

	while (!last_iteration) {
		int haves_added;
		struct object_id common_oid;
		int received_ready = 0;

		strbuf_reset(&req_buf);
		write_fetch_command_and_capabilities(&req_buf, server_options);

		packet_buf_write(&req_buf, "wait-for-done");

		haves_added = add_haves(&negotiator, &req_buf, &haves_to_send);
		in_vain += haves_added;
		if (!haves_added || (seen_ack && in_vain >= MAX_IN_VAIN))
			last_iteration = 1;

		/* Send request */
		packet_buf_flush(&req_buf);
		if (write_in_full(fd[1], req_buf.buf, req_buf.len) < 0)
			die_errno(_("unable to write request to remote"));

		/* Process ACKs/NAKs */
		process_section_header(&reader, "acknowledgments", 0);
		while (process_ack(&negotiator, &reader, &common_oid,
				   &received_ready)) {
			struct commit *commit = lookup_commit(the_repository,
							      &common_oid);
			if (commit) {
				timestamp_t generation;

				parse_commit_or_die(commit);
				commit->object.flags |= COMMON;
				generation = commit_graph_generation(commit);
				if (generation < min_generation)
					min_generation = generation;
			}
			in_vain = 0;
			seen_ack = 1;
			oidset_insert(acked_commits, &common_oid);
		}
		if (received_ready)
			die(_("unexpected 'ready' from remote"));
		else
			do_check_stateless_delimiter(stateless_rpc, &reader);
		if (can_all_from_reach_with_flag(&nt_object_array, COMMON,
						 REACH_SCRATCH, 0,
						 min_generation))
			last_iteration = 1;
	}
	clear_common_flag(acked_commits);
	strbuf_release(&req_buf);
}


// Source: fetch-pack.c
// Lines 1996-2085
