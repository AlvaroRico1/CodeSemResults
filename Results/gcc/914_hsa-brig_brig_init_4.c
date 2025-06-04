brig_init (void)
{
  brig_insn_count = 0;

  if (brig_initialized)
    return;

  brig_string_htab = new hash_table<brig_string_slot_hasher> (37);
  brig_data.init (BRIG_SECTION_DATA_NAME);
  brig_code.init (BRIG_SECTION_CODE_NAME);
  brig_operand.init (BRIG_SECTION_OPERAND_NAME);
  brig_initialized = true;

  struct BrigDirectiveModule moddir;
  memset (&moddir, 0, sizeof (moddir));
  moddir.base.byteCount = lendian16 (sizeof (moddir));

  char *modname;
  if (main_input_filename && *main_input_filename != '\0')
    {
      const char *part = strrchr (main_input_filename, '/');
      if (!part)
	part = main_input_filename;
      else
	part++;
      modname = concat ("&__hsa_module_", part, NULL);
      char *extension = strchr (modname, '.');
      if (extension)
	*extension = '\0';

      /* As in LTO mode, we have to emit a different module names.  */
      if (flag_ltrans)
	{
	  part = strrchr (asm_file_name, '/');
	  if (!part)
	    part = asm_file_name;
	  else
	    part++;
	  char *modname2;
	  modname2 = xasprintf ("%s_%s", modname, part);
	  free (modname);
	  modname = modname2;
	}

      hsa_sanitize_name (modname);
      moddir.name = brig_emit_string (modname);
      free (modname);
    }


// Source: hsa-brig.c
// Lines 465-512
