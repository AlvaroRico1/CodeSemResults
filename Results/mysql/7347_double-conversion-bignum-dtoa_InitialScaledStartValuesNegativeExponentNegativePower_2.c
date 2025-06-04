static void InitialScaledStartValuesNegativeExponentNegativePower(
    uint64_t significand, int exponent,
    int estimated_power, bool need_boundary_deltas,
    Bignum* numerator, Bignum* denominator,
    Bignum* delta_minus, Bignum* delta_plus) {
  // Instead of multiplying the denominator with 10^estimated_power we
  // multiply all values (numerator and deltas) by 10^-estimated_power.

  // Use numerator as temporary container for power_ten.
  Bignum* power_ten = numerator;
  power_ten->AssignPowerUInt16(10, -estimated_power);

  if (need_boundary_deltas) {
    // Since power_ten == numerator we must make a copy of 10^estimated_power
    // before we complete the computation of the numerator.
    // delta_plus = delta_minus = 10^estimated_power
    delta_plus->AssignBignum(*power_ten);
    delta_minus->AssignBignum(*power_ten);
  }

  // numerator = significand * 2 * 10^-estimated_power
  //  since v = significand * 2^exponent this is equivalent to
  // numerator = v * 10^-estimated_power * 2 * 2^-exponent.
  // Remember: numerator has been abused as power_ten. So no need to assign it
  //  to itself.
  DOUBLE_CONVERSION_ASSERT(numerator == power_ten);
  numerator->MultiplyByUInt64(significand);

  // denominator = 2 * 2^-exponent with exponent < 0.
  denominator->AssignUInt16(1);
  denominator->ShiftLeft(-exponent);

  if (need_boundary_deltas) {
    // Introduce a common denominator so that the deltas to the boundaries are
    // integers.
    numerator->ShiftLeft(1);
    denominator->ShiftLeft(1);
    // With this shift the boundaries have their correct value, since
    // delta_plus = 10^-estimated_power, and
    // delta_minus = 10^-estimated_power.
    // These assignments have been done earlier.
    // The adjustments if f == 2^p-1 (lower boundary is closer) are done later.
  }
}


// Source: double-conversion-bignum-dtoa.cpp
// Lines 498-541
