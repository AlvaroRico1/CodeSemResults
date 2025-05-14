// Source: curl/src/tool_easysrc.c
// Lines 175-175
void dumpeasysrc(struct GlobalConfig *config)
{
  struct curl_slist *ptr;
  char *o = config->libcurl;

  FILE *out;
  bool fopened = FALSE;
  if(strcmp(o, "-")) {
    out = fopen(o, FOPEN_WRITETEXT);
    fopened = TRUE;
  }
  else
    out = stdout;
  if(!out)
    warnf(config, "Failed to open %s to write libcurl code!\n", o);
  else {
    int i;
    const char *c;

    for(i = 0; ((c = srchead[i]) != NULL); i++)
      fprintf(out, "%s\n", c);

    /* Declare variables used for complex setopt values */
    if(easysrc_decl) {
      for(ptr = easysrc_decl->first; ptr; ptr = ptr->next)
        fprintf(out, "  %s\n", ptr->data);
    }

    /* Set up complex values for setopt calls */
    if(easysrc_data) {
      fprintf(out, "\n");

      for(ptr = easysrc_data->first; ptr; ptr = ptr->next)
        fprintf(out, "  %s\n", ptr->data);
    }

    fprintf(out, "\n");
    if(easysrc_code) {
      for(ptr = easysrc_code->first; ptr; ptr = ptr->next) {
        if(ptr->data[0]) {
          fprintf(out, "  %s\n", ptr->data);
        }
        else {
          fprintf(out, "\n");
        }
      }
    }

    if(easysrc_clean) {
      for(ptr = easysrc_clean->first; ptr; ptr = ptr->next)
        fprintf(out, "  %s\n", ptr->data);
    }

    for(i = 0; ((c = srcend[i]) != NULL); i++)
      fprintf(out, "%s\n", c);

    if(fopened)
      fclose(out);
  }

  easysrc_free();
}
