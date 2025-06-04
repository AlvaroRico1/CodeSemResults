static int begin_packet_write_state(NET *net, uchar command,
                                    const uchar *packet, size_t packet_len,
                                    const uchar *optional_prefix,
                                    size_t prefix_len) {
  DBUG_TRACE;
  size_t header_len = NET_HEADER_SIZE;
  if (net->compress) {
    header_len += NET_HEADER_SIZE + COMP_HEADER_SIZE;
  }
  NET_ASYNC *net_async = NET_ASYNC_DATA(net);
  size_t total_len = packet_len + prefix_len;
  bool include_command = (command < COM_END);
  if (include_command) {
    ++total_len;
  }
  size_t packet_count = 1 + total_len / MAX_PACKET_LENGTH;
  reset_packet_write_state(net);

  struct io_vec *vec;
  uchar *headers;
  uchar **compressed_buffers = nullptr;
  if (total_len < MAX_PACKET_LENGTH) {
    /*
      Most writes hit this case, ie, less than MAX_PACKET_LENGTH of
      query text.
    */
    vec = net_async->inline_async_write_vector;
    headers = net_async->inline_async_write_header;
  } else {
    /* Large query, create the vector and header buffer dynamically. */
    vec = (struct io_vec *)my_malloc(
        PSI_NOT_INSTRUMENTED, sizeof(struct io_vec) * (packet_count * 2 + 1),
        MYF(MY_ZEROFILL));
    if (!vec) {
      return 0;
    }

    headers =
        (uchar *)my_malloc(PSI_NOT_INSTRUMENTED,
                           packet_count * (header_len + 1), MYF(MY_ZEROFILL));
    if (!headers) {
      my_free(vec);
      return 0;
    }
  }
  /*
    Regardless of where vec and headers come from, these are what we
    feed to writev and populate below.
  */
  net_async->async_write_vector = vec;
  net_async->async_write_headers = headers;
  if (net->compress) {
    // Will need to hand compress and manage at most 1 buffer per packet
    compressed_buffers =
        (uchar **)my_malloc(key_memory_NET_compress_packet,
                            sizeof(uchar *) * packet_count, MYF(MY_ZEROFILL));
    if (!compressed_buffers) {
      reset_packet_write_state(net);
      return 0;
    }
  }
  net_async->compressed_write_buffers = compressed_buffers;

  /*
    We sneak the command into the first header, so the special casing
    below about packet_num == 0 relates to that.  This lets us avoid
    an extra allocation and copying the input buffers again.

    Every chunk of MAX_PACKET_LENGTH results in a header and a
    payload, so we have twice as many entries in the IO
    vector as we have packet_count.  The first packet may be prefixed with a
    small amount of data, so that one actually might
    consume *three* iovec entries.
  */
  for (size_t packet_num = 0; packet_num < packet_count; ++packet_num) {
    /*
      The first iovec contains the headers only and command if it is provided
    */
    uchar *buf = headers + packet_num * (header_len + 1);
    size_t bytes_queued = 0;
    (*vec).iov_base = buf;
    (*vec).iov_len = header_len;

    /*
     if using compression, add the compression header. Usually, we would rely on
     compress_packet to add compression headers, but here we assume
     that headers do not compress well due to their short length and send them
     as is by constructing our own packat and incrementing compress_pkt_nr
     manually.

     We don't compress the headers together with the payload because that
     would mean extra memcpy's to concatenate the buffers to pass into
     compress_packet.
    */
    if (net->compress) {
      size_t comp_packet_len = NET_HEADER_SIZE;
      if (packet_num == 0) {
        comp_packet_len += prefix_len + (include_command ? 1 : 0);
      }
      int3store(buf, comp_packet_len);
      buf[3] = (uchar)net->compress_pkt_nr++;
      /*
       The bytes in COMP_HEADER_SIZE are implicitly zero because they were
       zero filled. A zero length means that the contents are uncompressed.
      */
      buf += NET_HEADER_SIZE + COMP_HEADER_SIZE;
    }


// Source: net_serv.cc
// Lines 527-633
