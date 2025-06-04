Timestamp TimeUtil::GetCurrentTime() {
  int64_t seconds;
  int32_t nanos;
  CurrentTime(&seconds, &nanos);
  return CreateNormalized<Timestamp>(seconds, nanos);
}


// Source: time_util.cc
// Lines 171-176
