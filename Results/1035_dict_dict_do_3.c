// Source: curl/lib/dict.c
// Lines 176-186
static CURLcode dict_do(struct Curl_easy *data, bool *done)
{
  char *word;
  char *eword;
  char *ppath;
  char *database = NULL;
  char *strategy = NULL;
  char *nthdef = NULL; /* This is not part of the protocol, but required
                          by RFC 2229 */
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];

  char *path = data->state.up.path;

  *done = TRUE; /* unconditionally */

  if(conn->bits.user_passwd) {
    /* AUTH is missing */
  }

  if(strncasecompare(path, DICT_MATCH, sizeof(DICT_MATCH)-1) ||
     strncasecompare(path, DICT_MATCH2, sizeof(DICT_MATCH2)-1) ||
     strncasecompare(path, DICT_MATCH3, sizeof(DICT_MATCH3)-1)) {

    word = strchr(path, ':');
    if(word) {
      word++;
      database = strchr(word, ':');
      if(database) {
        *database++ = (char)0;
        strategy = strchr(database, ':');
        if(strategy) {
          *strategy++ = (char)0;
          nthdef = strchr(strategy, ':');
          if(nthdef) {
            *nthdef = (char)0;
          }
        }
      }
    }

    if(!word || (*word == (char)0)) {
      infof(data, "lookup word is missing");
      word = (char *)"default";
    }
    if(!database || (*database == (char)0)) {
      database = (char *)"!";
    }
    if(!strategy || (*strategy == (char)0)) {
      strategy = (char *)".";
    }

    eword = unescape_word(data, word);
    if(!eword)
      return CURLE_OUT_OF_MEMORY;

    result = sendf(sockfd, data,
                   "CLIENT " LIBCURL_NAME " " LIBCURL_VERSION "\r\n"
                   "MATCH "
                   "%s "    /* database */
                   "%s "    /* strategy */
                   "%s\r\n" /* word */
                   "QUIT\r\n",
                   database,
                   strategy,
                   eword);

    free(eword);

    if(result) {
      failf(data, "Failed sending DICT request");
      return result;
    }
    Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1); /* no upload */
  }
  else if(strncasecompare(path, DICT_DEFINE, sizeof(DICT_DEFINE)-1) ||
          strncasecompare(path, DICT_DEFINE2, sizeof(DICT_DEFINE2)-1) ||
          strncasecompare(path, DICT_DEFINE3, sizeof(DICT_DEFINE3)-1)) {

    word = strchr(path, ':');
    if(word) {
      word++;
      database = strchr(word, ':');
      if(database) {
        *database++ = (char)0;
        nthdef = strchr(database, ':');
        if(nthdef) {
          *nthdef = (char)0;
        }
      }
    }

    if(!word || (*word == (char)0)) {
      infof(data, "lookup word is missing");
      word = (char *)"default";
    }
    if(!database || (*database == (char)0)) {
      database = (char *)"!";
    }

    eword = unescape_word(data, word);
    if(!eword)
      return CURLE_OUT_OF_MEMORY;

    result = sendf(sockfd, data,
                   "CLIENT " LIBCURL_NAME " " LIBCURL_VERSION "\r\n"
                   "DEFINE "
                   "%s "     /* database */
                   "%s\r\n"  /* word */
                   "QUIT\r\n",
                   database,
                   eword);

    free(eword);

    if(result) {
      failf(data, "Failed sending DICT request");
      return result;
    }
    Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
  }
  else {

    ppath = strchr(path, '/');
    if(ppath) {
      int i;

      ppath++;
      for(i = 0; ppath[i]; i++) {
        if(ppath[i] == ':')
          ppath[i] = ' ';
      }
      result = sendf(sockfd, data,
                     "CLIENT " LIBCURL_NAME " " LIBCURL_VERSION "\r\n"
                     "%s\r\n"
                     "QUIT\r\n", ppath);
      if(result) {
        failf(data, "Failed sending DICT request");
        return result;
      }

      Curl_setup_transfer(data, FIRSTSOCKET, -1, FALSE, -1);
    }
  }

  return CURLE_OK;
}
