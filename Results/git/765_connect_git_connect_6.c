struct child_process *git_connect(int fd[2], const char *url,
				  const char *prog, int flags)
{
	char *hostandport, *path;
	struct child_process *conn;
	enum protocol protocol;
	enum protocol_version version = get_protocol_version_config();

	/*
	 * NEEDSWORK: If we are trying to use protocol v2 and we are planning
	 * to perform a push, then fallback to v0 since the client doesn't know
	 * how to push yet using v2.
	 */
	if (version == protocol_v2 && !strcmp("git-receive-pack", prog))
		version = protocol_v0;

	/* Without this we cannot rely on waitpid() to tell
	 * what happened to our children.
	 */
	signal(SIGCHLD, SIG_DFL);

	protocol = parse_connect_url(url, &hostandport, &path);
	if ((flags & CONNECT_DIAG_URL) && (protocol != PROTO_SSH)) {
		printf("Diag: url=%s\n", url ? url : "NULL");
		printf("Diag: protocol=%s\n", prot_name(protocol));
		printf("Diag: hostandport=%s\n", hostandport ? hostandport : "NULL");
		printf("Diag: path=%s\n", path ? path : "NULL");
		conn = NULL;
	} else if (protocol == PROTO_GIT) {
		conn = git_connect_git(fd, hostandport, path, prog, version, flags);
		conn->trace2_child_class = "transport/git";
	} else {
		struct strbuf cmd = STRBUF_INIT;
		const char *const *var;

		conn = xmalloc(sizeof(*conn));
		child_process_init(conn);

		if (looks_like_command_line_option(path))
			die(_("strange pathname '%s' blocked"), path);

		strbuf_addstr(&cmd, prog);
		strbuf_addch(&cmd, ' ');
		sq_quote_buf(&cmd, path);

		/* remove repo-local variables from the environment */
		for (var = local_repo_env; *var; var++)
			strvec_push(&conn->env_array, *var);

		conn->use_shell = 1;
		conn->in = conn->out = -1;
		if (protocol == PROTO_SSH) {
			char *ssh_host = hostandport;
			const char *port = NULL;
			transport_check_allowed("ssh");
			get_host_and_port(&ssh_host, &port);

			if (!port)
				port = get_port(ssh_host);

			if (flags & CONNECT_DIAG_URL) {
				printf("Diag: url=%s\n", url ? url : "NULL");
				printf("Diag: protocol=%s\n", prot_name(protocol));
				printf("Diag: userandhost=%s\n", ssh_host ? ssh_host : "NULL");
				printf("Diag: port=%s\n", port ? port : "NONE");
				printf("Diag: path=%s\n", path ? path : "NULL");

				free(hostandport);
				free(path);
				free(conn);
				strbuf_release(&cmd);
				return NULL;
			}
			conn->trace2_child_class = "transport/ssh";
			fill_ssh_args(conn, ssh_host, port, version, flags);
		} else {
			transport_check_allowed("file");
			conn->trace2_child_class = "transport/file";
			if (version > 0) {
				strvec_pushf(&conn->env_array,
					     GIT_PROTOCOL_ENVIRONMENT "=version=%d",
					     version);
			}
		}
		strvec_push(&conn->args, cmd.buf);

		if (start_command(conn))
			die(_("unable to fork"));

		fd[0] = conn->out; /* read from child's stdout */
		fd[1] = conn->in;  /* write to child's stdin */
		strbuf_release(&cmd);
	}


// Source: connect.c
// Lines 1353-1445
