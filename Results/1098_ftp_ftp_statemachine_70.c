// Source: curl/lib/ftp.c
// Lines 2668-2838
static CURLcode ftp_statemachine(struct Curl_easy *data,
                                 struct connectdata *conn)
{
  CURLcode result;
  curl_socket_t sock = conn->sock[FIRSTSOCKET];
  int ftpcode;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  struct pingpong *pp = &ftpc->pp;
  static const char ftpauth[][4]  = { "SSL", "TLS" };
  size_t nread = 0;

  if(pp->sendleft)
    return Curl_pp_flushsend(data, pp);

  result = ftp_readresp(data, sock, pp, &ftpcode, &nread);
  if(result)
    return result;

  if(ftpcode) {
    /* we have now received a full FTP server response */
    switch(ftpc->state) {
    case FTP_WAIT220:
      if(ftpcode == 230) {
        /* 230 User logged in - already! Take as 220 if TLS required. */
        if(data->set.use_ssl <= CURLUSESSL_TRY ||
           conn->bits.ftp_use_control_ssl)
          return ftp_state_user_resp(data, ftpcode, ftpc->state);
      }
      else if(ftpcode != 220) {
        failf(data, "Got a %03d ftp-server response when 220 was expected",
              ftpcode);
        return CURLE_WEIRD_SERVER_REPLY;
      }

      /* We have received a 220 response fine, now we proceed. */
#ifdef HAVE_GSSAPI
      if(data->set.krb) {
        /* If not anonymous login, try a secure login. Note that this
           procedure is still BLOCKING. */

        Curl_sec_request_prot(conn, "private");
        /* We set private first as default, in case the line below fails to
           set a valid level */
        Curl_sec_request_prot(conn, data->set.str[STRING_KRB_LEVEL]);

        if(Curl_sec_login(data, conn))
          infof(data, "Logging in with password in cleartext!");
        else
          infof(data, "Authentication successful");
      }
#endif

      if(data->set.use_ssl && !conn->bits.ftp_use_control_ssl) {
        /* We don't have a SSL/TLS control connection yet, but FTPS is
           requested. Try a FTPS connection now */

        ftpc->count3 = 0;
        switch(data->set.ftpsslauth) {
        case CURLFTPAUTH_DEFAULT:
        case CURLFTPAUTH_SSL:
          ftpc->count2 = 1; /* add one to get next */
          ftpc->count1 = 0;
          break;
        case CURLFTPAUTH_TLS:
          ftpc->count2 = -1; /* subtract one to get next */
          ftpc->count1 = 1;
          break;
        default:
          failf(data, "unsupported parameter to CURLOPT_FTPSSLAUTH: %d",
                (int)data->set.ftpsslauth);
          return CURLE_UNKNOWN_OPTION; /* we don't know what to do */
        }
        result = Curl_pp_sendf(data, &ftpc->pp, "AUTH %s",
                               ftpauth[ftpc->count1]);
        if(!result)
          state(data, FTP_AUTH);
      }
      else
        result = ftp_state_user(data, conn);
      break;

    case FTP_AUTH:
      /* we have gotten the response to a previous AUTH command */

      if(pp->cache_size)
        return CURLE_WEIRD_SERVER_REPLY; /* Forbid pipelining in response. */

      /* RFC2228 (page 5) says:
       *
       * If the server is willing to accept the named security mechanism,
       * and does not require any security data, it must respond with
       * reply code 234/334.
       */

      if((ftpcode == 234) || (ftpcode == 334)) {
        /* Curl_ssl_connect is BLOCKING */
        result = Curl_ssl_connect(data, conn, FIRSTSOCKET);
        if(!result) {
          conn->bits.ftp_use_data_ssl = FALSE; /* clear-text data */
          conn->bits.ftp_use_control_ssl = TRUE; /* SSL on control */
          result = ftp_state_user(data, conn);
        }
      }
      else if(ftpc->count3 < 1) {
        ftpc->count3++;
        ftpc->count1 += ftpc->count2; /* get next attempt */
        result = Curl_pp_sendf(data, &ftpc->pp, "AUTH %s",
                               ftpauth[ftpc->count1]);
        /* remain in this same state */
      }
      else {
        if(data->set.use_ssl > CURLUSESSL_TRY)
          /* we failed and CURLUSESSL_CONTROL or CURLUSESSL_ALL is set */
          result = CURLE_USE_SSL_FAILED;
        else
          /* ignore the failure and continue */
          result = ftp_state_user(data, conn);
      }
      break;

    case FTP_USER:
    case FTP_PASS:
      result = ftp_state_user_resp(data, ftpcode, ftpc->state);
      break;

    case FTP_ACCT:
      result = ftp_state_acct_resp(data, ftpcode);
      break;

    case FTP_PBSZ:
      result =
        Curl_pp_sendf(data, &ftpc->pp, "PROT %c",
                      data->set.use_ssl == CURLUSESSL_CONTROL ? 'C' : 'P');
      if(!result)
        state(data, FTP_PROT);
      break;

    case FTP_PROT:
      if(ftpcode/100 == 2)
        /* We have enabled SSL for the data connection! */
        conn->bits.ftp_use_data_ssl =
          (data->set.use_ssl != CURLUSESSL_CONTROL) ? TRUE : FALSE;
      /* FTP servers typically responds with 500 if they decide to reject
         our 'P' request */
      else if(data->set.use_ssl > CURLUSESSL_CONTROL)
        /* we failed and bails out */
        return CURLE_USE_SSL_FAILED;

      if(data->set.ftp_ccc) {
        /* CCC - Clear Command Channel
         */
        result = Curl_pp_sendf(data, &ftpc->pp, "%s", "CCC");
        if(!result)
          state(data, FTP_CCC);
      }
      else
        result = ftp_state_pwd(data, conn);
      break;

    case FTP_CCC:
      if(ftpcode < 500) {
        /* First shut down the SSL layer (note: this call will block) */
        result = Curl_ssl_shutdown(data, conn, FIRSTSOCKET);

        if(result)
          failf(data, "Failed to clear the command channel (CCC)");
      }
      if(!result)
        /* Then continue as normal */
        result = ftp_state_pwd(data, conn);
      break;

    case FTP_PWD:
      if(ftpcode == 257) {
        char *ptr = &data->state.buffer[4];  /* start on the first letter */
        const size_t buf_size = data->set.buffer_size;
        char *dir;
        bool entry_extracted = FALSE;

        dir = malloc(nread + 1);
        if(!dir)
          return CURLE_OUT_OF_MEMORY;

        /* Reply format is like
           257<space>[rubbish]"<directory-name>"<space><commentary> and the
           RFC959 says

           The directory name can contain any character; embedded
           double-quotes should be escaped by double-quotes (the
           "quote-doubling" convention).
        */

        /* scan for the first double-quote for non-standard responses */
        while(ptr < &data->state.buffer[buf_size]
              && *ptr != '\n' && *ptr != '\0' && *ptr != '"')
          ptr++;

        if('\"' == *ptr) {
          /* it started good */
          char *store;
          ptr++;
          for(store = dir; *ptr;) {
            if('\"' == *ptr) {
              if('\"' == ptr[1]) {
                /* "quote-doubling" */
                *store = ptr[1];
                ptr++;
              }
              else {
                /* end of path */
                entry_extracted = TRUE;
                break; /* get out of this loop */
              }
            }
            else
              *store = *ptr;
            store++;
            ptr++;
          }
          *store = '\0'; /* null-terminate */
        }
        if(entry_extracted) {
          /* If the path name does not look like an absolute path (i.e.: it
             does not start with a '/'), we probably need some server-dependent
             adjustments. For example, this is the case when connecting to
             an OS400 FTP server: this server supports two name syntaxes,
             the default one being incompatible with standard paths. In
             addition, this server switches automatically to the regular path
             syntax when one is encountered in a command: this results in
             having an entrypath in the wrong syntax when later used in CWD.
               The method used here is to check the server OS: we do it only
             if the path name looks strange to minimize overhead on other
             systems. */

          if(!ftpc->server_os && dir[0] != '/') {
            result = Curl_pp_sendf(data, &ftpc->pp, "%s", "SYST");
            if(result) {
              free(dir);
              return result;
            }
            Curl_safefree(ftpc->entrypath);
            ftpc->entrypath = dir; /* remember this */
            infof(data, "Entry path is '%s'", ftpc->entrypath);
            /* also save it where getinfo can access it: */
            data->state.most_recent_ftp_entrypath = ftpc->entrypath;
            state(data, FTP_SYST);
            break;
          }

          Curl_safefree(ftpc->entrypath);
          ftpc->entrypath = dir; /* remember this */
          infof(data, "Entry path is '%s'", ftpc->entrypath);
          /* also save it where getinfo can access it: */
          data->state.most_recent_ftp_entrypath = ftpc->entrypath;
        }
        else {
          /* couldn't get the path */
          free(dir);
          infof(data, "Failed to figure out path");
        }
      }
