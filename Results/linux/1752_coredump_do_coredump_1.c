void do_coredump(const kernel_siginfo_t *siginfo)
{
	struct core_state core_state;
	struct core_name cn;
	struct mm_struct *mm = current->mm;
	struct linux_binfmt * binfmt;
	const struct cred *old_cred;
	struct cred *cred;
	int retval = 0;
	int ispipe;
	size_t *argv = NULL;
	int argc = 0;
	struct files_struct *displaced;
	/* require nonrelative corefile path and be extra careful */
	bool need_suid_safe = false;
	bool core_dumped = false;
	static atomic_t core_dump_count = ATOMIC_INIT(0);
	struct coredump_params cprm = {
		.siginfo = siginfo,
		.regs = signal_pt_regs(),
		.limit = rlimit(RLIMIT_CORE),
		/*
		 * We must use the same mm->flags while dumping core to avoid
		 * inconsistency of bit flags, since this flag is not protected
		 * by any locks.
		 */
		.mm_flags = mm->flags,
	};

	audit_core_dumps(siginfo->si_signo);

	binfmt = mm->binfmt;
	if (!binfmt || !binfmt->core_dump)
		goto fail;
	if (!__get_dumpable(cprm.mm_flags))
		goto fail;

	cred = prepare_creds();
	if (!cred)
		goto fail;
	/*
	 * We cannot trust fsuid as being the "true" uid of the process
	 * nor do we know its entire history. We only know it was tainted
	 * so we dump it as root in mode 2, and only into a controlled
	 * environment (pipe handler or fully qualified path).
	 */
	if (__get_dumpable(cprm.mm_flags) == SUID_DUMP_ROOT) {
		/* Setuid core dump mode */
		cred->fsuid = GLOBAL_ROOT_UID;	/* Dump root private */
		need_suid_safe = true;
	}

	retval = coredump_wait(siginfo->si_signo, &core_state);
	if (retval < 0)
		goto fail_creds;

	old_cred = override_creds(cred);

	ispipe = format_corename(&cn, &cprm, &argv, &argc);

	if (ispipe) {
		int argi;
		int dump_count;
		char **helper_argv;
		struct subprocess_info *sub_info;

		if (ispipe < 0) {
			printk(KERN_WARNING "format_corename failed\n");
			printk(KERN_WARNING "Aborting core\n");
			goto fail_unlock;
		}

		if (cprm.limit == 1) {
			/* See umh_pipe_setup() which sets RLIMIT_CORE = 1.
			 *
			 * Normally core limits are irrelevant to pipes, since
			 * we're not writing to the file system, but we use
			 * cprm.limit of 1 here as a special value, this is a
			 * consistent way to catch recursive crashes.
			 * We can still crash if the core_pattern binary sets
			 * RLIM_CORE = !1, but it runs as root, and can do
			 * lots of stupid things.
			 *
			 * Note that we use task_tgid_vnr here to grab the pid
			 * of the process group leader.  That way we get the
			 * right pid if a thread in a multi-threaded
			 * core_pattern process dies.
			 */
			printk(KERN_WARNING
				"Process %d(%s) has RLIMIT_CORE set to 1\n",
				task_tgid_vnr(current), current->comm);
			printk(KERN_WARNING "Aborting core\n");
			goto fail_unlock;
		}
		cprm.limit = RLIM_INFINITY;

		dump_count = atomic_inc_return(&core_dump_count);
		if (core_pipe_limit && (core_pipe_limit < dump_count)) {
			printk(KERN_WARNING "Pid %d(%s) over core_pipe_limit\n",
			       task_tgid_vnr(current), current->comm);
			printk(KERN_WARNING "Skipping core dump\n");
			goto fail_dropcount;
		}

		helper_argv = kmalloc_array(argc + 1, sizeof(*helper_argv),
					    GFP_KERNEL);
		if (!helper_argv) {
			printk(KERN_WARNING "%s failed to allocate memory\n",
			       __func__);
			goto fail_dropcount;
		}
		for (argi = 0; argi < argc; argi++)
			helper_argv[argi] = cn.corename + argv[argi];
		helper_argv[argi] = NULL;

		retval = -ENOMEM;
		sub_info = call_usermodehelper_setup(helper_argv[0],
						helper_argv, NULL, GFP_KERNEL,
						umh_pipe_setup, NULL, &cprm);
		if (sub_info)
			retval = call_usermodehelper_exec(sub_info,
							  UMH_WAIT_EXEC);

		kfree(helper_argv);
		if (retval) {
			printk(KERN_INFO "Core dump to |%s pipe failed\n",
			       cn.corename);
			goto close_fail;
		}
	} else {
		struct inode *inode;
		int open_flags = O_CREAT | O_RDWR | O_NOFOLLOW |
				 O_LARGEFILE | O_EXCL;

		if (cprm.limit < binfmt->min_coredump)
			goto fail_unlock;

		if (need_suid_safe && cn.corename[0] != '/') {
			printk(KERN_WARNING "Pid %d(%s) can only dump core "\
				"to fully qualified path!\n",
				task_tgid_vnr(current), current->comm);
			printk(KERN_WARNING "Skipping core dump\n");
			goto fail_unlock;
		}

		/*
		 * Unlink the file if it exists unless this is a SUID
		 * binary - in that case, we're running around with root
		 * privs and don't want to unlink another user's coredump.
		 */
		if (!need_suid_safe) {
			/*
			 * If it doesn't exist, that's fine. If there's some
			 * other problem, we'll catch it at the filp_open().
			 */
			do_unlinkat(AT_FDCWD, getname_kernel(cn.corename));
		}

		/*
		 * There is a race between unlinking and creating the
		 * file, but if that causes an EEXIST here, that's
		 * fine - another process raced with us while creating
		 * the corefile, and the other process won. To userspace,
		 * what matters is that at least one of the two processes
		 * writes its coredump successfully, not which one.
		 */
		if (need_suid_safe) {
			/*
			 * Using user namespaces, normal user tasks can change
			 * their current->fs->root to point to arbitrary
			 * directories. Since the intention of the "only dump
			 * with a fully qualified path" rule is to control where
			 * coredumps may be placed using root privileges,
			 * current->fs->root must not be used. Instead, use the
			 * root directory of init_task.
			 */
			struct path root;

			task_lock(&init_task);
			get_fs_root(init_task.fs, &root);
			task_unlock(&init_task);
			cprm.file = file_open_root(root.dentry, root.mnt,
				cn.corename, open_flags, 0600);
			path_put(&root);
		} else {
			cprm.file = filp_open(cn.corename, open_flags, 0600);
		}
		if (IS_ERR(cprm.file))
			goto fail_unlock;

		inode = file_inode(cprm.file);
		if (inode->i_nlink > 1)
			goto close_fail;
		if (d_unhashed(cprm.file->f_path.dentry))
			goto close_fail;
		/*
		 * AK: actually i see no reason to not allow this for named
		 * pipes etc, but keep the previous behaviour for now.
		 */
		if (!S_ISREG(inode->i_mode))
			goto close_fail;
		/*
		 * Don't dump core if the filesystem changed owner or mode
		 * of the file during file creation. This is an issue when
		 * a process dumps core while its cwd is e.g. on a vfat
		 * filesystem.
		 */
		if (!uid_eq(inode->i_uid, current_fsuid()))
			goto close_fail;
		if ((inode->i_mode & 0677) != 0600)
			goto close_fail;
		if (!(cprm.file->f_mode & FMODE_CAN_WRITE))
			goto close_fail;
		if (do_truncate(cprm.file->f_path.dentry, 0, 0, cprm.file))
			goto close_fail;
	}


// Source: coredump.c
// Lines 565-780
