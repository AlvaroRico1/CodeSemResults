static void __init fpu__init_parse_early_param(void)
{
	char arg[32];
	char *argptr = arg;
	int bit;

#ifdef CONFIG_X86_32
	if (cmdline_find_option_bool(boot_command_line, "no387"))
#ifdef CONFIG_MATH_EMULATION
		setup_clear_cpu_cap(X86_FEATURE_FPU);
#else
		pr_err("Option 'no387' required CONFIG_MATH_EMULATION enabled.\n");
#endif

	if (cmdline_find_option_bool(boot_command_line, "nofxsr"))
		setup_clear_cpu_cap(X86_FEATURE_FXSR);
#endif

	if (cmdline_find_option_bool(boot_command_line, "noxsave"))
		setup_clear_cpu_cap(X86_FEATURE_XSAVE);

	if (cmdline_find_option_bool(boot_command_line, "noxsaveopt"))
		setup_clear_cpu_cap(X86_FEATURE_XSAVEOPT);

	if (cmdline_find_option_bool(boot_command_line, "noxsaves"))
		setup_clear_cpu_cap(X86_FEATURE_XSAVES);

	if (cmdline_find_option(boot_command_line, "clearcpuid", arg,
				sizeof(arg)) &&
	    get_option(&argptr, &bit) &&
	    bit >= 0 &&
	    bit < NCAPINTS * 32)
		setup_clear_cpu_cap(bit);
}


// Source: init.c
// Lines 243-276
