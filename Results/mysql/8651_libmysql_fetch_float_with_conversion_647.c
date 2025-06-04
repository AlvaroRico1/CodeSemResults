static void fetch_float_with_conversion(MYSQL_BIND *param, MYSQL_FIELD *field,
                                        double value, my_gcvt_arg_type type) {
  uchar *buffer = pointer_cast<uchar *>(param->buffer);
  double val64 = (value < 0 ? -floor(-value) : floor(value));

  switch (param->buffer_type) {
    case MYSQL_TYPE_NULL: /* do nothing */
      break;
    case MYSQL_TYPE_TINY:
      /*
        We need to _store_ data in the buffer before the truncation check to
        workaround Intel FPU executive precision feature.
        (See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=323 for details)
        Sic: AFAIU it does not guarantee to work.
      */
      if (param->is_unsigned) {
        if (value < 0.0) {
          *param->error = true;
          break;
        }
        *buffer = (uint8)value;
      } else {
        *buffer = (int8)value;
      }
      *param->error = val64 != (param->is_unsigned ? (double)((uint8)*buffer)
                                                   : (double)((int8)*buffer));
      break;
    case MYSQL_TYPE_SHORT:
      if (param->is_unsigned) {
        if (value < 0.0) {
          *param->error = true;
          break;
        }
        ushort data = (ushort)value;
        shortstore(buffer, data);
      } else {
        short data = (short)value;
        shortstore(buffer, data);
      }
      *param->error =
          val64 != (param->is_unsigned ? (double)(*(ushort *)buffer)
                                       : (double)(*(short *)buffer));
      break;
    case MYSQL_TYPE_LONG:
      if (param->is_unsigned) {
        if (value < 0.0) {
          *param->error = true;
          break;
        }
        uint32 data = (uint32)value;
        longstore(buffer, data);
      } else {
        int32 data = (int32)value;
        longstore(buffer, data);
      }
      *param->error =
          val64 != (param->is_unsigned ? (double)(*(uint32 *)buffer)
                                       : (double)(*(int32 *)buffer));
      break;
    case MYSQL_TYPE_LONGLONG:
      if (param->is_unsigned) {
        if (value < 0.0) {
          *param->error = true;
          break;
        }
        ulonglong data = (ulonglong)value;
        longlongstore(buffer, data);
      } else {
        longlong data = (longlong)value;
        longlongstore(buffer, data);
      }
      *param->error =
          val64 != (param->is_unsigned ? ulonglong2double(*(ulonglong *)buffer)
                                       : (double)(*(longlong *)buffer));
      break;
    case MYSQL_TYPE_FLOAT: {
      float data = (float)value;
      floatstore(buffer, data);
      *param->error = (*(float *)buffer) != value;
      break;
    }


// Source: libmysql.cc
// Lines 2947-3027
