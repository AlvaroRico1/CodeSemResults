bool TimeUtil::FromString(const std::string& value, Timestamp* timestamp) {
  int64_t seconds;
  int32_t nanos;
  if (!ParseTime(value, &seconds, &nanos)) {
    return false;
  }
  *timestamp = CreateNormalized<Timestamp>(seconds, nanos);
  return true;
}


// Source: time_util.cc
// Lines 161-169
