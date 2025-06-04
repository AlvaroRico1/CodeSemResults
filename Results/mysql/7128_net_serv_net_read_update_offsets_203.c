static ulong net_read_update_offsets(NET *net, size_t start_of_packet,
                                     size_t first_packet_offset,
                                     size_t buf_length,
                                     uint multi_byte_packet) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("multi_byte_packet: %u, first_packet_offset : %zu, "
                      "start_of_packet : %zu",
                      multi_byte_packet, first_packet_offset, start_of_packet));
  net->read_pos = net->buff + first_packet_offset + NET_HEADER_SIZE;
  net->buf_length = buf_length;
  net->remain_in_buf = (ulong)(buf_length - start_of_packet);
  ulong len = ((ulong)(start_of_packet - first_packet_offset) -
               NET_HEADER_SIZE - multi_byte_packet);
  if (net->remain_in_buf) {
    /*
      If multi byte packet is non-zero then there is a zero length
      packet at read_pos[len]. Adding the size of one header
      reads the correct byte that will later be replaced. Guarded
      to avoid buffer overflow. If remain_buf = 0 then the char
      wont be restored anyway
    */
    net->save_char = net->read_pos[len + multi_byte_packet];
  }
  net->read_pos[len] = '\0';  // Safeguard for mysql_use_result.
  DBUG_PRINT("info", ("len :%lu, net->remain_in_buf : %lu, net->read_pos: %d",
                      len, net->remain_in_buf, *(net->read_pos)));
  return len;
}


// Source: net_serv.cc
// Lines 1882-1909
