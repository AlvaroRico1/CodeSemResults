// Source: curl/lib/smtp.c
// Lines 1766-1773
static CURLcode smtp_parse_address(struct Curl_easy *data, const char *fqma,
                                   char **address, struct hostname *host)
{
  CURLcode result = CURLE_OK;
  size_t length;

  /* Duplicate the fully qualified email address so we can manipulate it,
     ensuring it doesn't contain the delimiters if specified */
  char *dup = strdup(fqma[0] == '<' ? fqma + 1  : fqma);
  if(!dup)
    return CURLE_OUT_OF_MEMORY;

  length = strlen(dup);
  if(length) {
    if(dup[length - 1] == '>')
      dup[length - 1] = '\0';
  }

  /* Extract the host name from the address (if we can) */
  host->name = strpbrk(dup, "@");
  if(host->name) {
    *host->name = '\0';
    host->name = host->name + 1;

    /* Attempt to convert the host name to IDN ACE */
    (void) Curl_idnconvert_hostname(data, host);

    /* If Curl_idnconvert_hostname() fails then we shall attempt to continue
       and send the host name using UTF-8 rather than as 7-bit ACE (which is
       our preference) */
  }

  /* Extract the local address from the mailbox */
  *address = dup;

  return result;
}
