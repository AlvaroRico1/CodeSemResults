client_exec(const char *shell, const char *shellcmd)
{
	const char	*name, *ptr;
	char		*argv0;

	log_debug("shell %s, command %s", shell, shellcmd);

	ptr = strrchr(shell, '/');
	if (ptr != NULL && *(ptr + 1) != '\0')
		name = ptr + 1;
	else
		name = shell;
	if (client_flags & CLIENT_LOGIN)
		xasprintf(&argv0, "-%s", name);
	else
		xasprintf(&argv0, "%s", name);
	setenv("SHELL", shell, 1);

	proc_clear_signals(client_proc, 1);

	setblocking(STDIN_FILENO, 1);
	setblocking(STDOUT_FILENO, 1);
	setblocking(STDERR_FILENO, 1);
	closefrom(STDERR_FILENO + 1);

	execl(shell, argv0, "-c", shellcmd, (char *) NULL);
	fatal("execl failed");
}


// Source: client.c
// Lines 494-521
