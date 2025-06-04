static void do_check_stateless_delimiter(int stateless_rpc,
					 struct packet_reader *reader)
{
	check_stateless_delimiter(stateless_rpc, reader,
				  _("git fetch-pack: expected response end packet"));
}

static struct ref *do_fetch_pack_v2(struct fetch_pack_args *args,
				    int fd[2],
				    const struct ref *orig_ref,
				    struct ref **sought, int nr_sought,
				    struct oid_array *shallows,
				    struct shallow_info *si,
				    struct string_list *pack_lockfiles)
{
	struct repository *r = the_repository;
	struct ref *ref = copy_ref_list(orig_ref);
	enum fetch_state state = FETCH_CHECK_LOCAL;
	struct oidset common = OIDSET_INIT;
	struct packet_reader reader;
	int in_vain = 0, negotiation_started = 0;
	int haves_to_send = INITIAL_FLUSH;
	struct fetch_negotiator negotiator_alloc;
	struct fetch_negotiator *negotiator;
	int seen_ack = 0;
	struct object_id common_oid;
	int received_ready = 0;
	struct string_list packfile_uris = STRING_LIST_INIT_DUP;
	int i;
	struct strvec index_pack_args = STRVEC_INIT;

	negotiator = &negotiator_alloc;
	fetch_negotiator_init(r, negotiator);

	packet_reader_init(&reader, fd[0], NULL, 0,
			   PACKET_READ_CHOMP_NEWLINE |
			   PACKET_READ_DIE_ON_ERR_PACKET);
	if (git_env_bool("GIT_TEST_SIDEBAND_ALL", 1) &&
	    server_supports_feature("fetch", "sideband-all", 0)) {
		reader.use_sideband = 1;
		reader.me = "fetch-pack";
	}

	while (state != FETCH_DONE) {
		switch (state) {
		case FETCH_CHECK_LOCAL:
			sort_ref_list(&ref, ref_compare_name);
			QSORT(sought, nr_sought, cmp_ref_by_name);

			/* v2 supports these by default */
			allow_unadvertised_object_request |= ALLOW_REACHABLE_SHA1;
			use_sideband = 2;
			if (args->depth > 0 || args->deepen_since || args->deepen_not)
				args->deepen = 1;

			/* Filter 'ref' by 'sought' and those that aren't local */
			mark_complete_and_common_ref(negotiator, args, &ref);
			filter_refs(args, &ref, sought, nr_sought);
			if (everything_local(args, &ref))
				state = FETCH_DONE;
			else
				state = FETCH_SEND_REQUEST;

			mark_tips(negotiator, args->negotiation_tips);
			for_each_cached_alternate(negotiator,
						  insert_one_alternate_object);
			break;
		case FETCH_SEND_REQUEST:
			if (!negotiation_started) {
				negotiation_started = 1;
				trace2_region_enter("fetch-pack",
						    "negotiation_v2",
						    the_repository);
			}
			if (send_fetch_request(negotiator, fd[1], args, ref,
					       &common,
					       &haves_to_send, &in_vain,
					       reader.use_sideband,
					       seen_ack))
				state = FETCH_GET_PACK;
			else
				state = FETCH_PROCESS_ACKS;
			break;
		case FETCH_PROCESS_ACKS:
			/* Process ACKs/NAKs */
			process_section_header(&reader, "acknowledgments", 0);
			while (process_ack(negotiator, &reader, &common_oid,
					   &received_ready)) {
				in_vain = 0;
				seen_ack = 1;
				oidset_insert(&common, &common_oid);
			}
			if (received_ready) {
				/*
				 * Don't check for response delimiter; get_pack() will
				 * read the rest of this response.
				 */
				state = FETCH_GET_PACK;
			} else {
				do_check_stateless_delimiter(args->stateless_rpc, &reader);
				state = FETCH_SEND_REQUEST;
			}
			break;
		case FETCH_GET_PACK:
			trace2_region_leave("fetch-pack",
					    "negotiation_v2",
					    the_repository);
			/* Check for shallow-info section */
			if (process_section_header(&reader, "shallow-info", 1))
				receive_shallow_info(args, &reader, shallows, si);

			if (process_section_header(&reader, "wanted-refs", 1))
				receive_wanted_refs(&reader, sought, nr_sought);

			/* get the pack(s) */
			if (process_section_header(&reader, "packfile-uris", 1))
				receive_packfile_uris(&reader, &packfile_uris);
			process_section_header(&reader, "packfile", 0);

			/*
			 * this is the final request we'll make of the server;
			 * do a half-duplex shutdown to indicate that they can
			 * hang up as soon as the pack is sent.
			 */
			close(fd[1]);
			fd[1] = -1;

			if (get_pack(args, fd, pack_lockfiles,
				     packfile_uris.nr ? &index_pack_args : NULL,
				     sought, nr_sought, &fsck_options.gitmodules_found))
				die(_("git fetch-pack: fetch failed."));
			do_check_stateless_delimiter(args->stateless_rpc, &reader);

			state = FETCH_DONE;
			break;
		case FETCH_DONE:
			continue;
		}
	}

	for (i = 0; i < packfile_uris.nr; i++) {
		int j;
		struct child_process cmd = CHILD_PROCESS_INIT;
		char packname[GIT_MAX_HEXSZ + 1];
		const char *uri = packfile_uris.items[i].string +
			the_hash_algo->hexsz + 1;

		strvec_push(&cmd.args, "http-fetch");
		strvec_pushf(&cmd.args, "--packfile=%.*s",
			     (int) the_hash_algo->hexsz,
			     packfile_uris.items[i].string);
		for (j = 0; j < index_pack_args.nr; j++)
			strvec_pushf(&cmd.args, "--index-pack-arg=%s",
				     index_pack_args.v[j]);
		strvec_push(&cmd.args, uri);
		cmd.git_cmd = 1;
		cmd.no_stdin = 1;
		cmd.out = -1;
		if (start_command(&cmd))
			die("fetch-pack: unable to spawn http-fetch");

		if (read_in_full(cmd.out, packname, 5) < 0 ||
		    memcmp(packname, "keep\t", 5))
			die("fetch-pack: expected keep then TAB at start of http-fetch output");

		if (read_in_full(cmd.out, packname,
				 the_hash_algo->hexsz + 1) < 0 ||
		    packname[the_hash_algo->hexsz] != '\n')
			die("fetch-pack: expected hash then LF at end of http-fetch output");

		packname[the_hash_algo->hexsz] = '\0';

		parse_gitmodules_oids(cmd.out, &fsck_options.gitmodules_found);

		close(cmd.out);

		if (finish_command(&cmd))
			die("fetch-pack: unable to finish http-fetch");

		if (memcmp(packfile_uris.items[i].string, packname,
			   the_hash_algo->hexsz))
			die("fetch-pack: pack downloaded from %s does not match expected hash %.*s",
			    uri, (int) the_hash_algo->hexsz,
			    packfile_uris.items[i].string);

		string_list_append_nodup(pack_lockfiles,
					 xstrfmt("%s/pack/pack-%s.keep",
						 get_object_directory(),
						 packname));
	}
	string_list_clear(&packfile_uris, 0);
	strvec_clear(&index_pack_args);

	if (fsck_finish(&fsck_options))
		die("fsck failed");

	if (negotiator)
		negotiator->release(negotiator);

	oidset_clear(&common);
	return ref;
}


// Source: fetch-pack.c
// Lines 1541-1742
