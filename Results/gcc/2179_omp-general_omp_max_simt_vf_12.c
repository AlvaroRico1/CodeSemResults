omp_max_simt_vf (void)
{
  if (!optimize)
    return 0;
  if (ENABLE_OFFLOADING)
    for (const char *c = getenv ("OFFLOAD_TARGET_NAMES"); c;)
      {
	if (!strncmp (c, "nvptx", strlen ("nvptx")))
	  return 32;
	else if ((c = strchr (c, ':')))
	  c++;
      }
  return 0;
}


// Source: omp-general.c
// Lines 598-611
