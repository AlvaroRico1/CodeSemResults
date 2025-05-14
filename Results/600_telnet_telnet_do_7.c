// Source: curl/lib/telnet.c
// Lines 1255-1258
static CURLcode telnet_do(struct Curl_easy *data, bool *done)
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  curl_socket_t sockfd = conn->sock[FIRSTSOCKET];
#ifdef USE_WINSOCK
  WSAEVENT event_handle;
  WSANETWORKEVENTS events;
  HANDLE stdin_handle;
  HANDLE objs[2];
  DWORD  obj_count;
  DWORD  wait_timeout;
  DWORD readfile_read;
  int err;
#else
  timediff_t interval_ms;
  struct pollfd pfd[2];
  int poll_cnt;
  curl_off_t total_dl = 0;
  curl_off_t total_ul = 0;
#endif
  ssize_t nread;
  struct curltime now;
  bool keepon = TRUE;
  char *buf = data->state.buffer;
  struct TELNET *tn;

  *done = TRUE; /* unconditionally */

  result = init_telnet(data);
  if(result)
    return result;

  tn = data->req.p.telnet;

  result = check_telnet_options(data);
  if(result)
    return result;

#ifdef USE_WINSOCK
  /* We want to wait for both stdin and the socket. Since
  ** the select() function in winsock only works on sockets
  ** we have to use the WaitForMultipleObjects() call.
  */

  /* First, create a sockets event object */
  event_handle = WSACreateEvent();
  if(event_handle == WSA_INVALID_EVENT) {
    failf(data, "WSACreateEvent failed (%d)", SOCKERRNO);
    return CURLE_FAILED_INIT;
  }

  /* Tell winsock what events we want to listen to */
  if(WSAEventSelect(sockfd, event_handle, FD_READ|FD_CLOSE) == SOCKET_ERROR) {
    WSACloseEvent(event_handle);
    return CURLE_OK;
  }

  /* The get the Windows file handle for stdin */
  stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

  /* Create the list of objects to wait for */
  objs[0] = event_handle;
  objs[1] = stdin_handle;

  /* If stdin_handle is a pipe, use PeekNamedPipe() method to check it,
     else use the old WaitForMultipleObjects() way */
  if(GetFileType(stdin_handle) == FILE_TYPE_PIPE ||
     data->set.is_fread_set) {
    /* Don't wait for stdin_handle, just wait for event_handle */
    obj_count = 1;
    /* Check stdin_handle per 100 milliseconds */
    wait_timeout = 100;
  }
  else {
    obj_count = 2;
    wait_timeout = 1000;
  }

  /* Keep on listening and act on events */
  while(keepon) {
    const DWORD buf_size = (DWORD)data->set.buffer_size;
    DWORD waitret = WaitForMultipleObjects(obj_count, objs,
                                           FALSE, wait_timeout);
    switch(waitret) {

    case WAIT_TIMEOUT:
    {
      for(;;) {
        if(data->set.is_fread_set) {
          size_t n;
          /* read from user-supplied method */
          n = data->state.fread_func(buf, 1, buf_size, data->state.in);
          if(n == CURL_READFUNC_ABORT) {
            keepon = FALSE;
            result = CURLE_READ_ERROR;
            break;
          }

          if(n == CURL_READFUNC_PAUSE)
            break;

          if(n == 0)                        /* no bytes */
            break;

          /* fall through with number of bytes read */
          readfile_read = (DWORD)n;
        }
        else {
          /* read from stdin */
          if(!PeekNamedPipe(stdin_handle, NULL, 0, NULL,
                            &readfile_read, NULL)) {
            keepon = FALSE;
            result = CURLE_READ_ERROR;
            break;
          }

          if(!readfile_read)
            break;

          if(!ReadFile(stdin_handle, buf, buf_size,
                       &readfile_read, NULL)) {
            keepon = FALSE;
            result = CURLE_READ_ERROR;
            break;
          }
        }

        result = send_telnet_data(data, buf, readfile_read);
        if(result) {
          keepon = FALSE;
          break;
        }
      }
    }
    break;

    case WAIT_OBJECT_0 + 1:
    {
      if(!ReadFile(stdin_handle, buf, buf_size,
                   &readfile_read, NULL)) {
        keepon = FALSE;
        result = CURLE_READ_ERROR;
        break;
      }

      result = send_telnet_data(data, buf, readfile_read);
      if(result) {
        keepon = FALSE;
        break;
      }
    }
    break;

    case WAIT_OBJECT_0:
    {
      events.lNetworkEvents = 0;
      if(WSAEnumNetworkEvents(sockfd, event_handle, &events) == SOCKET_ERROR) {
        err = SOCKERRNO;
        if(err != EINPROGRESS) {
          infof(data, "WSAEnumNetworkEvents failed (%d)", err);
          keepon = FALSE;
          result = CURLE_READ_ERROR;
        }
        break;
      }
      if(events.lNetworkEvents & FD_READ) {
        /* read data from network */
        result = Curl_read(data, sockfd, buf, data->set.buffer_size, &nread);
        /* read would've blocked. Loop again */
        if(result == CURLE_AGAIN)
          break;
        /* returned not-zero, this an error */
        else if(result) {
          keepon = FALSE;
          break;
        }
        /* returned zero but actually received 0 or less here,
           the server closed the connection and we bail out */
        else if(nread <= 0) {
          keepon = FALSE;
          break;
        }

        result = telrcv(data, (unsigned char *) buf, nread);
        if(result) {
          keepon = FALSE;
          break;
        }

        /* Negotiate if the peer has started negotiating,
           otherwise don't. We don't want to speak telnet with
           non-telnet servers, like POP or SMTP. */
        if(tn->please_negotiate && !tn->already_negotiated) {
          negotiate(data);
          tn->already_negotiated = 1;
        }
      }
      if(events.lNetworkEvents & FD_CLOSE) {
        keepon = FALSE;
      }
    }
    break;

    }

    if(data->set.timeout) {
      now = Curl_now();
      if(Curl_timediff(now, conn->created) >= data->set.timeout) {
        failf(data, "Time-out");
        result = CURLE_OPERATION_TIMEDOUT;
        keepon = FALSE;
      }
    }
  }

  /* We called WSACreateEvent, so call WSACloseEvent */
  if(!WSACloseEvent(event_handle)) {
    infof(data, "WSACloseEvent failed (%d)", SOCKERRNO);
  }
#else
  pfd[0].fd = sockfd;
  pfd[0].events = POLLIN;

  if(data->set.is_fread_set) {
    poll_cnt = 1;
    interval_ms = 100; /* poll user-supplied read function */
  }
  else {
    /* really using fread, so infile is a FILE* */
    pfd[1].fd = fileno((FILE *)data->state.in);
    pfd[1].events = POLLIN;
    poll_cnt = 2;
    interval_ms = 1 * 1000;
  }

  while(keepon) {
    switch(Curl_poll(pfd, poll_cnt, interval_ms)) {
    case -1:                    /* error, stop reading */
      keepon = FALSE;
      continue;
    case 0:                     /* timeout */
      pfd[0].revents = 0;
      pfd[1].revents = 0;
      /* FALLTHROUGH */
    default:                    /* read! */
      if(pfd[0].revents & POLLIN) {
        /* read data from network */
        result = Curl_read(data, sockfd, buf, data->set.buffer_size, &nread);
        /* read would've blocked. Loop again */
        if(result == CURLE_AGAIN)
          break;
        /* returned not-zero, this an error */
        if(result) {
          keepon = FALSE;
          break;
        }
        /* returned zero but actually received 0 or less here,
           the server closed the connection and we bail out */
        else if(nread <= 0) {
          keepon = FALSE;
          break;
        }

        total_dl += nread;
        Curl_pgrsSetDownloadCounter(data, total_dl);
        result = telrcv(data, (unsigned char *)buf, nread);
        if(result) {
          keepon = FALSE;
          break;
        }

        /* Negotiate if the peer has started negotiating,
           otherwise don't. We don't want to speak telnet with
           non-telnet servers, like POP or SMTP. */
        if(tn->please_negotiate && !tn->already_negotiated) {
          negotiate(data);
          tn->already_negotiated = 1;
        }
      }

      nread = 0;
      if(poll_cnt == 2) {
        if(pfd[1].revents & POLLIN) { /* read from in file */
          nread = read(pfd[1].fd, buf, data->set.buffer_size);
        }
      }
      else {
        /* read from user-supplied method */
        nread = (int)data->state.fread_func(buf, 1, data->set.buffer_size,
                                            data->state.in);
        if(nread == CURL_READFUNC_ABORT) {
          keepon = FALSE;
          break;
        }
        if(nread == CURL_READFUNC_PAUSE)
          break;
      }

      if(nread > 0) {
        result = send_telnet_data(data, buf, nread);
        if(result) {
          keepon = FALSE;
          break;
        }
        total_ul += nread;
        Curl_pgrsSetUploadCounter(data, total_ul);
      }
      else if(nread < 0)
        keepon = FALSE;

      break;
    } /* poll switch statement */

    if(data->set.timeout) {
      now = Curl_now();
      if(Curl_timediff(now, conn->created) >= data->set.timeout) {
        failf(data, "Time-out");
        result = CURLE_OPERATION_TIMEDOUT;
        keepon = FALSE;
      }
    }

    if(Curl_pgrsUpdate(data)) {
      result = CURLE_ABORTED_BY_CALLBACK;
      break;
    }
  }
#endif
  /* mark this as "no further transfer wanted" */
  Curl_setup_transfer(data, -1, -1, FALSE, -1);

  return result;
}
