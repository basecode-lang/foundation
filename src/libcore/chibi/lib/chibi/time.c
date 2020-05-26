/* Automatically generated by chibi-ffi; version: 0.4 */

#include <chibi/eval.h>

#include <time.h>

#include <sys/time.h>

#include <sys/resource.h>
/*
types: (rusage timezone timeval tm)
enums: ()
*/

sexp sexp_get_resource_usage_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  int err = 0;
  struct rusage* tmp1;
  sexp res;
  sexp_gc_var1(res1);
  if (! sexp_exact_integerp(arg0))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg0);
  sexp_gc_preserve1(ctx, res1);
  tmp1 = (struct rusage*) sexp_calloc(sexp_context_alloc(ctx), 1, 1 + sizeof(tmp1[0]));
  err = getrusage(sexp_sint_value(arg0), tmp1);
  if (err) {
  res = SEXP_FALSE;
  } else {
  res1 = sexp_make_cpointer(ctx, sexp_unbox_fixnum(sexp_opcode_arg2_type(self)), tmp1, SEXP_FALSE, 1);
  res = res1;
  }
  sexp_gc_release1(ctx);
  return res;
}

sexp sexp_time_3e_string_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  char *err;
  char tmp1[64];
  sexp res;
  sexp_gc_var1(res1);
  if (! (sexp_pointerp(arg0) && (sexp_pointer_tag(arg0) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), arg0);
  sexp_gc_preserve1(ctx, res1);
  err = asctime_r((struct tm*)sexp_cpointer_value(arg0), tmp1);
  if (!err) {
  res = SEXP_FALSE;
  } else {
  res1 = sexp_c_string(ctx, tmp1, -1);
  res = res1;
  }
  sexp_gc_release1(ctx);
  return res;
}

sexp sexp_seconds_3e_string_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  char *err;
  char tmp1[64];
  time_t tmp0;
  sexp res;
  sexp_gc_var1(res1);
  if (! sexp_exact_integerp(arg0))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg0);
  sexp_gc_preserve1(ctx, res1);
  tmp0 = sexp_unshift_epoch(sexp_uint_value(arg0));
  err = ctime_r(&tmp0, tmp1);
  if (!err) {
  res = SEXP_FALSE;
  } else {
  res1 = sexp_c_string(ctx, tmp1, -1);
  res = res1;
  }
  sexp_gc_release1(ctx);
  return res;
}

sexp sexp_time_3e_seconds_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  if (! (sexp_pointerp(arg0) && (sexp_pointer_tag(arg0) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), arg0);
  res = sexp_make_integer(ctx, sexp_shift_epoch(mktime((struct tm*)sexp_cpointer_value(arg0))));
  return res;
}

sexp sexp_seconds_3e_time_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  void *err;
  struct tm* tmp1;
  time_t tmp0;
  sexp res;
  sexp_gc_var1(res1);
  if (! sexp_exact_integerp(arg0))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg0);
  sexp_gc_preserve1(ctx, res1);
  tmp0 = sexp_unshift_epoch(sexp_uint_value(arg0));
  tmp1 = (struct tm*) sexp_calloc(sexp_context_alloc(ctx), 1, 1 + sizeof(tmp1[0]));
  err = localtime_r(&tmp0, tmp1);
  if (!err) {
  res = SEXP_FALSE;
  } else {
  res1 = sexp_make_cpointer(ctx, sexp_unbox_fixnum(sexp_opcode_arg2_type(self)), tmp1, SEXP_FALSE, 1);
  res = res1;
  }
  sexp_gc_release1(ctx);
  return res;
}

sexp sexp_set_time_of_day_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  int err = 0;
  sexp res;
  if (! (sexp_pointerp(arg0) && (sexp_pointer_tag(arg0) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), arg0);
  if (! ((sexp_pointerp(arg1) && (sexp_pointer_tag(arg1) == sexp_unbox_fixnum(sexp_opcode_arg2_type(self)))) || sexp_not(arg1)))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg2_type(self)), arg1);
  err = settimeofday((struct timeval*)sexp_cpointer_value(arg0), (struct timezone*)sexp_cpointer_maybe_null_value(arg1));
  if (err) {
  res = SEXP_FALSE;
  } else {
  res = SEXP_TRUE;
  }
  return res;
}

sexp sexp_get_time_of_day_stub (sexp ctx, sexp self, sexp_sint_t n) {
  int err = 0;
  struct timeval* tmp0;
  struct timezone* tmp1;
  sexp_gc_var3(res, res0, res1);
  sexp_gc_preserve3(ctx, res, res0, res1);
  tmp0 = (struct timeval*) sexp_calloc(sexp_context_alloc(ctx), 1, 1 + sizeof(tmp0[0]));
  tmp1 = (struct timezone*) sexp_calloc(sexp_context_alloc(ctx), 1, 1 + sizeof(tmp1[0]));
  err = gettimeofday(tmp0, tmp1);
  if (err) {
  res = SEXP_FALSE;
  } else {
  res0 = sexp_make_cpointer(ctx, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), tmp0, SEXP_FALSE, 1);
  res1 = sexp_make_cpointer(ctx, sexp_unbox_fixnum(sexp_opcode_arg2_type(self)), tmp1, SEXP_FALSE, 1);
  res = SEXP_NULL;
  sexp_push(ctx, res, res1);
  sexp_push(ctx, res, res0);
  }
  sexp_gc_release3(ctx);
  return res;
}

sexp sexp_current_seconds_stub (sexp ctx, sexp self, sexp_sint_t n) {
  sexp res;
  res = sexp_make_integer(ctx, sexp_shift_epoch(time(NULL)));
  return res;
}

sexp sexp_rusage_get_ru_utime (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_cpointer(ctx, sexp_unbox_fixnum(sexp_opcode_return_type(self)), &((struct rusage*)sexp_cpointer_value(x))->ru_utime, x, 0);
}

sexp sexp_rusage_get_ru_stime (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_cpointer(ctx, sexp_unbox_fixnum(sexp_opcode_return_type(self)), &((struct rusage*)sexp_cpointer_value(x))->ru_stime, x, 0);
}

sexp sexp_rusage_get_ru_maxrss (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct rusage*)sexp_cpointer_value(x))->ru_maxrss);
}

sexp sexp_timezone_get_tz_minuteswest (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct timezone*)sexp_cpointer_value(x))->tz_minuteswest);
}

sexp sexp_timezone_get_tz_dsttime (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct timezone*)sexp_cpointer_value(x))->tz_dsttime);
}

sexp sexp_make_timeval_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  struct timeval* r;
  sexp_gc_var1(res);
  sexp_gc_preserve1(ctx, res);
  res = sexp_alloc_tagged(ctx, sexp_sizeof(cpointer), sexp_unbox_fixnum(sexp_opcode_return_type(self)));
  sexp_cpointer_value(res) = sexp_calloc(sexp_context_alloc(ctx), 1, sizeof(struct timeval));
  r = (struct timeval*) sexp_cpointer_value(res);
  memset(r, 0, sizeof(struct timeval));
  sexp_freep(res) = 1;
  r->tv_sec = sexp_unshift_epoch(sexp_uint_value(arg0));
  r->tv_usec = sexp_sint_value(arg1);
  sexp_gc_release1(ctx);
  return res;
}

sexp sexp_timeval_get_tv_sec (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, sexp_shift_epoch(((struct timeval*)sexp_cpointer_value(x))->tv_sec));
}

sexp sexp_timeval_get_tv_usec (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct timeval*)sexp_cpointer_value(x))->tv_usec);
}

sexp sexp_make_tm_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2, sexp arg3, sexp arg4, sexp arg5, sexp arg6) {
  struct tm* r;
  sexp_gc_var1(res);
  sexp_gc_preserve1(ctx, res);
  res = sexp_alloc_tagged(ctx, sexp_sizeof(cpointer), sexp_unbox_fixnum(sexp_opcode_return_type(self)));
  sexp_cpointer_value(res) = sexp_calloc(sexp_context_alloc(ctx), 1, sizeof(struct tm));
  r = (struct tm*) sexp_cpointer_value(res);
  memset(r, 0, sizeof(struct tm));
  sexp_freep(res) = 1;
  r->tm_sec = sexp_sint_value(arg0);
  r->tm_min = sexp_sint_value(arg1);
  r->tm_hour = sexp_sint_value(arg2);
  r->tm_mday = sexp_sint_value(arg3);
  r->tm_mon = sexp_sint_value(arg4);
  r->tm_year = sexp_sint_value(arg5);
  r->tm_isdst = sexp_sint_value(arg6);
  sexp_gc_release1(ctx);
  return res;
}

sexp sexp_tm_get_tm_sec (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_sec);
}

sexp sexp_tm_get_tm_min (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_min);
}

sexp sexp_tm_get_tm_hour (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_hour);
}

sexp sexp_tm_get_tm_mday (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_mday);
}

sexp sexp_tm_get_tm_mon (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_mon);
}

sexp sexp_tm_get_tm_year (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_year);
}

sexp sexp_tm_get_tm_wday (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_wday);
}

sexp sexp_tm_get_tm_yday (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_yday);
}

sexp sexp_tm_get_tm_isdst (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_isdst);
}

sexp sexp_tm_get_tm_zone (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_c_string(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_zone, -1);
}

sexp sexp_tm_get_tm_gmtoff (sexp ctx, sexp self, sexp_sint_t n, sexp x) {
  if (! (sexp_pointerp(x) && (sexp_pointer_tag(x) == sexp_unbox_fixnum(sexp_opcode_arg1_type(self)))))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg1_type(self)), x);
  return sexp_make_integer(ctx, ((struct tm*)sexp_cpointer_value(x))->tm_gmtoff);
}


sexp sexp_init_library (sexp ctx, sexp self, sexp_sint_t n, sexp env, const char* version, const sexp_abi_identifier_t abi) {
  sexp sexp_rusage_type_obj;
  sexp sexp_timezone_type_obj;
  sexp sexp_timeval_type_obj;
  sexp sexp_tm_type_obj;
  sexp_gc_var3(name, tmp, op);
  if (!(sexp_version_compatible(ctx, version, sexp_version)
        && sexp_abi_compatible(ctx, abi, SEXP_ABI_IDENTIFIER)))
    return SEXP_ABI_ERROR;
  sexp_gc_preserve3(ctx, name, tmp, op);
  name = sexp_intern(ctx, "resource-usage/children", 23);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, RUSAGE_CHILDREN));
  name = sexp_intern(ctx, "resource-usage/self", 19);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, RUSAGE_SELF));
  name = sexp_c_string(ctx, "rusage", -1);
  sexp_rusage_type_obj = sexp_register_c_type(ctx, name, sexp_finalize_c_type);
  tmp = sexp_string_to_symbol(ctx, name);
  sexp_env_define(ctx, env, tmp, sexp_rusage_type_obj);
  sexp_type_slots(sexp_rusage_type_obj) = SEXP_NULL;
  sexp_push(ctx, sexp_type_slots(sexp_rusage_type_obj), sexp_intern(ctx, "ru_maxrss", -1));
  sexp_push(ctx, sexp_type_slots(sexp_rusage_type_obj), sexp_intern(ctx, "ru_stime", -1));
  sexp_push(ctx, sexp_type_slots(sexp_rusage_type_obj), sexp_intern(ctx, "ru_utime", -1));
  sexp_type_getters(sexp_rusage_type_obj) = sexp_make_vector(ctx, SEXP_THREE, SEXP_FALSE);
  sexp_type_setters(sexp_rusage_type_obj) = sexp_make_vector(ctx, SEXP_THREE, SEXP_FALSE);
  tmp = sexp_make_type_predicate(ctx, name, sexp_rusage_type_obj);
  name = sexp_intern(ctx, "rusage?", 7);
  sexp_env_define(ctx, env, name, tmp);
  name = sexp_c_string(ctx, "timezone", -1);
  sexp_timezone_type_obj = sexp_register_c_type(ctx, name, sexp_finalize_c_type);
  tmp = sexp_string_to_symbol(ctx, name);
  sexp_env_define(ctx, env, tmp, sexp_timezone_type_obj);
  sexp_type_slots(sexp_timezone_type_obj) = SEXP_NULL;
  sexp_push(ctx, sexp_type_slots(sexp_timezone_type_obj), sexp_intern(ctx, "tz_dsttime", -1));
  sexp_push(ctx, sexp_type_slots(sexp_timezone_type_obj), sexp_intern(ctx, "tz_minuteswest", -1));
  sexp_type_getters(sexp_timezone_type_obj) = sexp_make_vector(ctx, SEXP_TWO, SEXP_FALSE);
  sexp_type_setters(sexp_timezone_type_obj) = sexp_make_vector(ctx, SEXP_TWO, SEXP_FALSE);
  tmp = sexp_make_type_predicate(ctx, name, sexp_timezone_type_obj);
  name = sexp_intern(ctx, "timezone?", 9);
  sexp_env_define(ctx, env, name, tmp);
  name = sexp_c_string(ctx, "timeval", -1);
  sexp_timeval_type_obj = sexp_register_c_type(ctx, name, sexp_finalize_c_type);
  tmp = sexp_string_to_symbol(ctx, name);
  sexp_env_define(ctx, env, tmp, sexp_timeval_type_obj);
  sexp_type_slots(sexp_timeval_type_obj) = SEXP_NULL;
  sexp_push(ctx, sexp_type_slots(sexp_timeval_type_obj), sexp_intern(ctx, "tv_usec", -1));
  sexp_push(ctx, sexp_type_slots(sexp_timeval_type_obj), sexp_intern(ctx, "tv_sec", -1));
  sexp_type_getters(sexp_timeval_type_obj) = sexp_make_vector(ctx, SEXP_TWO, SEXP_FALSE);
  sexp_type_setters(sexp_timeval_type_obj) = sexp_make_vector(ctx, SEXP_TWO, SEXP_FALSE);
  tmp = sexp_make_type_predicate(ctx, name, sexp_timeval_type_obj);
  name = sexp_intern(ctx, "timeval?", 8);
  sexp_env_define(ctx, env, name, tmp);
  name = sexp_c_string(ctx, "tm", -1);
  sexp_tm_type_obj = sexp_register_c_type(ctx, name, sexp_finalize_c_type);
  tmp = sexp_string_to_symbol(ctx, name);
  sexp_env_define(ctx, env, tmp, sexp_tm_type_obj);
  sexp_type_slots(sexp_tm_type_obj) = SEXP_NULL;
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_gmtoff", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_zone", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_isdst", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_yday", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_wday", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_year", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_mon", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_mday", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_hour", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_min", -1));
  sexp_push(ctx, sexp_type_slots(sexp_tm_type_obj), sexp_intern(ctx, "tm_sec", -1));
  sexp_type_getters(sexp_tm_type_obj) = sexp_make_vector(ctx, sexp_make_fixnum(11), SEXP_FALSE);
  sexp_type_setters(sexp_tm_type_obj) = sexp_make_vector(ctx, sexp_make_fixnum(11), SEXP_FALSE);
  tmp = sexp_make_type_predicate(ctx, name, sexp_tm_type_obj);
  name = sexp_intern(ctx, "tm?", 3);
  sexp_env_define(ctx, env, name, tmp);
  op = sexp_define_foreign(ctx, env, "time-offset", 1, sexp_tm_get_tm_gmtoff);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_TEN, op);
  op = sexp_define_foreign(ctx, env, "time-timezone-name", 1, sexp_tm_get_tm_zone);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_STRING);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_NINE, op);
  op = sexp_define_foreign(ctx, env, "time-dst?", 1, sexp_tm_get_tm_isdst);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_EIGHT, op);
  op = sexp_define_foreign(ctx, env, "time-day-of-year", 1, sexp_tm_get_tm_yday);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_SEVEN, op);
  op = sexp_define_foreign(ctx, env, "time-day-of-week", 1, sexp_tm_get_tm_wday);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_SIX, op);
  op = sexp_define_foreign(ctx, env, "time-year", 1, sexp_tm_get_tm_year);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_FIVE, op);
  op = sexp_define_foreign(ctx, env, "time-month", 1, sexp_tm_get_tm_mon);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_FOUR, op);
  op = sexp_define_foreign(ctx, env, "time-day", 1, sexp_tm_get_tm_mday);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_THREE, op);
  op = sexp_define_foreign(ctx, env, "time-hour", 1, sexp_tm_get_tm_hour);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_TWO, op);
  op = sexp_define_foreign(ctx, env, "time-minute", 1, sexp_tm_get_tm_min);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_ONE, op);
  op = sexp_define_foreign(ctx, env, "time-second", 1, sexp_tm_get_tm_sec);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_tm_type_obj))) sexp_vector_set(sexp_type_getters(sexp_tm_type_obj), SEXP_ZERO, op);
  op = sexp_define_foreign(ctx, env, "make-tm", 7, sexp_make_tm_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_argn_type(op) = sexp_make_vector(ctx, SEXP_FOUR, sexp_make_fixnum(SEXP_OBJECT));
    sexp_vector_set(sexp_opcode_argn_type(op), SEXP_ZERO, sexp_make_fixnum(SEXP_FIXNUM));
    sexp_vector_set(sexp_opcode_argn_type(op), SEXP_ONE, sexp_make_fixnum(SEXP_FIXNUM));
    sexp_vector_set(sexp_opcode_argn_type(op), SEXP_TWO, sexp_make_fixnum(SEXP_FIXNUM));
    sexp_vector_set(sexp_opcode_argn_type(op), SEXP_THREE, sexp_make_fixnum(SEXP_FIXNUM));
  }
  op = sexp_define_foreign(ctx, env, "timeval-microseconds", 1, sexp_timeval_get_tv_usec);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timeval_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_timeval_type_obj))) sexp_vector_set(sexp_type_getters(sexp_timeval_type_obj), SEXP_ONE, op);
  op = sexp_define_foreign(ctx, env, "timeval-seconds", 1, sexp_timeval_get_tv_sec);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timeval_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_timeval_type_obj))) sexp_vector_set(sexp_type_getters(sexp_timeval_type_obj), SEXP_ZERO, op);
  op = sexp_define_foreign(ctx, env, "make-timeval", 2, sexp_make_timeval_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timeval_type_obj));
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "timezone-dst-time", 1, sexp_timezone_get_tz_dsttime);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timezone_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_timezone_type_obj))) sexp_vector_set(sexp_type_getters(sexp_timezone_type_obj), SEXP_ONE, op);
  op = sexp_define_foreign(ctx, env, "timezone-offset", 1, sexp_timezone_get_tz_minuteswest);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timezone_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_timezone_type_obj))) sexp_vector_set(sexp_type_getters(sexp_timezone_type_obj), SEXP_ZERO, op);
  op = sexp_define_foreign(ctx, env, "resource-usage-max-rss", 1, sexp_rusage_get_ru_maxrss);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_rusage_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_rusage_type_obj))) sexp_vector_set(sexp_type_getters(sexp_rusage_type_obj), SEXP_TWO, op);
  op = sexp_define_foreign(ctx, env, "resource-usage-system-time", 1, sexp_rusage_get_ru_stime);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timeval_type_obj));
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_rusage_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_rusage_type_obj))) sexp_vector_set(sexp_type_getters(sexp_rusage_type_obj), SEXP_ONE, op);
  op = sexp_define_foreign(ctx, env, "resource-usage-time", 1, sexp_rusage_get_ru_utime);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timeval_type_obj));
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_rusage_type_obj));
  }
  if (sexp_vectorp(sexp_type_getters(sexp_rusage_type_obj))) sexp_vector_set(sexp_type_getters(sexp_rusage_type_obj), SEXP_ZERO, op);
  op = sexp_define_foreign_opt(ctx, env, "get-resource-usage", 1, sexp_get_resource_usage_stub, sexp_make_integer(ctx, RUSAGE_SELF));
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_OBJECT);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_rusage_type_obj));
  }
  op = sexp_define_foreign(ctx, env, "time->string", 1, sexp_time_3e_string_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_STRING);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_CHAR);
  }
  op = sexp_define_foreign(ctx, env, "seconds->string", 1, sexp_seconds_3e_string_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_STRING);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_CHAR);
  }
  op = sexp_define_foreign(ctx, env, "time->seconds", 1, sexp_time_3e_seconds_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  op = sexp_define_foreign(ctx, env, "seconds->time", 1, sexp_seconds_3e_time_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_OBJECT);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_tm_type_obj));
  }
  op = sexp_define_foreign_opt(ctx, env, "set-time-of-day!", 2, sexp_set_time_of_day_x_stub, SEXP_FALSE);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_OBJECT);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timeval_type_obj));
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timezone_type_obj));
  }
  op = sexp_define_foreign(ctx, env, "get-time-of-day", 0, sexp_get_time_of_day_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_OBJECT);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timeval_type_obj));
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_timezone_type_obj));
  }
  op = sexp_define_foreign(ctx, env, "current-seconds", 0, sexp_current_seconds_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_OBJECT);
  }
  sexp_gc_release3(ctx);
  return SEXP_VOID;
}

