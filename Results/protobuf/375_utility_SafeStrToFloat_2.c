bool SafeStrToFloat(StringPiece str, float* value) {
  double double_value;
  if (!safe_strtod(str, &double_value)) {
    return false;
  }

  if (std::isinf(double_value) || std::isnan(double_value)) return false;

  // Fail if the value is not representable in float.
  if (double_value > std::numeric_limits<float>::max() ||
      double_value < -std::numeric_limits<float>::max()) {
    return false;
  }

  *value = static_cast<float>(double_value);
  return true;
}


// Source: utility.cc
// Lines 395-411
