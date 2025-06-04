inline my_ulonglong net_field_length_ll_safe(MYSQL *mysql, uchar **packet,
                                             ulong packet_length,
                                             bool *is_error) {
  size_t sizeof_len = net_field_length_size(*packet);
  DBUG_EXECUTE_IF("simulate_bad_packet",
                  { *packet = *packet + packet_length + 1000000L; });
  if (!buffer_check_remaining(mysql, *packet, packet_length, sizeof_len)) {
    *is_error = true;
    return 0;
  }

  *is_error = false;
  return net_field_length_ll(packet);
}


// Source: client.cc
// Lines 740-753
