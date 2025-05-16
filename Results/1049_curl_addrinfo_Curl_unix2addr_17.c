struct Curl_addrinfo *Curl_unix2addr(const char *path, bool *longpath,
                                     bool abstract)
{
  struct Curl_addrinfo *ai;
  struct sockaddr_un *sa_un;
  size_t path_len;

  *longpath = FALSE;

  ai = calloc(1, sizeof(struct Curl_addrinfo) + sizeof(struct sockaddr_un));
  if(!ai)
    return NULL;
  ai->ai_addr = (void *)((char *)ai + sizeof(struct Curl_addrinfo));

  sa_un = (void *) ai->ai_addr;
  sa_un->sun_family = AF_UNIX;

  /* sun_path must be able to store the NUL-terminated path */
  path_len = strlen(path) + 1;
  if(path_len > sizeof(sa_un->sun_path)) {
    free(ai);
    *longpath = TRUE;
    return NULL;
  }

  ai->ai_family = AF_UNIX;
  ai->ai_socktype = SOCK_STREAM; /* assume reliable transport for HTTP */
  ai->ai_addrlen = (curl_socklen_t)
    ((offsetof(struct sockaddr_un, sun_path) + path_len) & 0x7FFFFFFF);

  /* Abstract Unix domain socket have NULL prefix instead of suffix */
  if(abstract)
    memcpy(sa_un->sun_path + 1, path, path_len - 1);
  else
    memcpy(sa_un->sun_path, path, path_len); /* copy NUL byte */

  return ai;
}


// Source: curl_addrinfo.c
// Lines 470-507
