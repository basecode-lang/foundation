/* Automatically generated by chibi-ffi; version: 0.4 */

#include <chibi/eval.h>

int uvector_of(sexp uv, int etype) {
  return sexp_uvectorp(uv) && sexp_uvector_type(uv) == etype;
}

int u1vector_ref(sexp uv, int i) {
  return sexp_bit_ref(uv, i);
}
void u1vector_set(sexp uv, int i, int b) {
  sexp_bit_set(uv, i, b);
}

signed char s8vector_ref(signed char* uv, int i) {
  return uv[i];
}
void s8vector_set(signed char* uv, int i, signed char v) {
  uv[i] = v;
}

unsigned short u16vector_ref(unsigned short* uv, int i) {
  return uv[i];
}
void u16vector_set(unsigned short* uv, int i, unsigned short v) {
  uv[i] = v;
}

short s16vector_ref(short* uv, int i) {
  return uv[i];
}
void s16vector_set(short* uv, int i, short v) {
  uv[i] = v;
}

unsigned int u32vector_ref(unsigned int* uv, int i) {
  return uv[i];
}
void u32vector_set(unsigned int* uv, int i, unsigned int v) {
  uv[i] = v;
}

int s32vector_ref(int* uv, int i) {
  return uv[i];
}
void s32vector_set(int* uv, int i, int v) {
  uv[i] = v;
}

unsigned long u64vector_ref(unsigned long* uv, int i) {
  return uv[i];
}
void u64vector_set(unsigned long* uv, int i, unsigned long v) {
  uv[i] = v;
}

long s64vector_ref(long* uv, int i) {
  return uv[i];
}
void s64vector_set(long* uv, int i, long v) {
  uv[i] = v;
}

float f32vector_ref(float* uv, int i) {
  return uv[i];
}
void f32vector_set(float* uv, int i, float v) {
  uv[i] = v;
}

double f64vector_ref(double* uv, int i) {
  return uv[i];
}
void f64vector_set(double* uv, int i, double v) {
  uv[i] = v;
}

sexp c64vector_ref(sexp ctx, float* uv, int i) {
  sexp_gc_var3(real, imag, res);
  sexp_gc_preserve3(ctx, real, imag, res);
  real = sexp_make_flonum(ctx, uv[i*2]);
  imag = sexp_make_flonum(ctx, uv[i*2 + 1]);
  res = sexp_make_complex(ctx, real, imag);
  sexp_gc_release3(ctx);
  return res;
}
void c64vector_set(float* uv, int i, sexp v) {
  uv[i*2] = sexp_to_double(sexp_real_part(v));
  uv[i*2 + 1] = sexp_to_double(sexp_imag_part(v));
}

sexp c128vector_ref(sexp ctx, double* uv, int i) {
  sexp_gc_var3(real, imag, res);
  sexp_gc_preserve3(ctx, real, imag, res);
  real = sexp_make_flonum(ctx, uv[i*2]);
  imag = sexp_make_flonum(ctx, uv[i*2 + 1]);
  res = sexp_make_complex(ctx, real, imag);
  sexp_gc_release3(ctx);
  return res;
}
void c128vector_set(double* uv, int i, sexp v) {
  uv[i*2] = sexp_to_double(sexp_real_part(v));
  uv[i*2 + 1] = sexp_to_double(sexp_imag_part(v));
}

/*
types: ()
enums: ()
*/

sexp sexp_c128vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_c128vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = ((c128vector_set(((double*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), arg2)), SEXP_VOID);
  return res;
}

sexp sexp_c128vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_c128vectorp(arg1))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = c128vector_ref(ctx, ((double*) sexp_uvector_data(arg1)), sexp_sint_value(arg2));
  return res;
}

sexp sexp_c64vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_c64vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = ((c64vector_set(((float*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), arg2)), SEXP_VOID);
  return res;
}

sexp sexp_c64vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_c64vectorp(arg1))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = c64vector_ref(ctx, ((float*) sexp_uvector_data(arg1)), sexp_sint_value(arg2));
  return res;
}

sexp sexp_f64vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_f64vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_flonump(arg2))
    return sexp_type_exception(ctx, self, SEXP_FLONUM, arg2);
  res = ((f64vector_set(((double*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_flonum_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_f64vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_f64vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_flonum(ctx, f64vector_ref(((double*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_f32vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_f32vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_flonump(arg2))
    return sexp_type_exception(ctx, self, SEXP_FLONUM, arg2);
  res = ((f32vector_set(((float*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_flonum_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_f32vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_f32vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_flonum(ctx, f32vector_ref(((float*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_s64vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_s64vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((s64vector_set(((sexp_sint_t*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_sint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_s64vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_s64vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_integer(ctx, s64vector_ref(((sexp_sint_t*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_u64vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_u64vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((u64vector_set(((sexp_uint_t*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_uint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_u64vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_u64vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_unsigned_integer(ctx, u64vector_ref(((sexp_uint_t*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_s32vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_s32vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((s32vector_set(((signed int*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_sint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_s32vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_s32vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_integer(ctx, s32vector_ref(((signed int*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_u32vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_u32vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((u32vector_set(((unsigned int*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_uint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_u32vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_u32vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_unsigned_integer(ctx, u32vector_ref(((unsigned int*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_s16vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_s16vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((s16vector_set(((signed short*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_sint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_s16vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_s16vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_integer(ctx, s16vector_ref(((signed short*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_u16vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_u16vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((u16vector_set(((unsigned short*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_uint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_u16vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_u16vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_unsigned_integer(ctx, u16vector_ref(((unsigned short*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_s8vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_s8vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((s8vector_set(((signed char*) sexp_uvector_data(arg0)), sexp_sint_value(arg1), sexp_sint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_s8vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_s8vectorp(arg0))
    return sexp_type_exception(ctx, self, SEXP_UNIFORM_VECTOR, arg0);
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_integer(ctx, s8vector_ref(((signed char*) sexp_uvector_data(arg0)), sexp_sint_value(arg1)));
  return res;
}

sexp sexp_u1vector_set_x_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1, sexp arg2) {
  sexp res;
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  if (! sexp_exact_integerp(arg2))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg2);
  res = ((u1vector_set(arg0, sexp_sint_value(arg1), sexp_sint_value(arg2))), SEXP_VOID);
  return res;
}

sexp sexp_u1vector_ref_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0, sexp arg1) {
  sexp res;
  if (! sexp_exact_integerp(arg1))
    return sexp_type_exception(ctx, self, SEXP_FIXNUM, arg1);
  res = sexp_make_integer(ctx, u1vector_ref(arg0, sexp_sint_value(arg1)));
  return res;
}

sexp sexp_c128vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_C128));
  return res;
}

sexp sexp_c64vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_C64));
  return res;
}

sexp sexp_f64vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_F64));
  return res;
}

sexp sexp_f32vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_F32));
  return res;
}

sexp sexp_s64vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_S64));
  return res;
}

sexp sexp_u64vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_U64));
  return res;
}

sexp sexp_s32vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_S32));
  return res;
}

sexp sexp_u32vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_U32));
  return res;
}

sexp sexp_s16vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_S16));
  return res;
}

sexp sexp_u16vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_U16));
  return res;
}

sexp sexp_s8vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_S8));
  return res;
}

sexp sexp_u1vector_p_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_boolean(uvector_of(arg0, SEXP_U1));
  return res;
}

sexp sexp_uvector_length_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  res = sexp_make_integer(ctx, sexp_uvector_length(arg0));
  return res;
}


sexp sexp_init_library (sexp ctx, sexp self, sexp_sint_t n, sexp env, const char* version, const sexp_abi_identifier_t abi) {
  sexp_gc_var3(name, tmp, op);
  if (!(sexp_version_compatible(ctx, version, sexp_version)
        && sexp_abi_compatible(ctx, abi, SEXP_ABI_IDENTIFIER)))
    return SEXP_ABI_ERROR;
  sexp_gc_preserve3(ctx, name, tmp, op);
  name = sexp_intern(ctx, "SEXP_C128", 9);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_C128));
  name = sexp_intern(ctx, "SEXP_C64", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_C64));
  name = sexp_intern(ctx, "SEXP_F64", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_F64));
  name = sexp_intern(ctx, "SEXP_F32", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_F32));
  name = sexp_intern(ctx, "SEXP_U64", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_U64));
  name = sexp_intern(ctx, "SEXP_S64", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_S64));
  name = sexp_intern(ctx, "SEXP_U32", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_U32));
  name = sexp_intern(ctx, "SEXP_S32", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_S32));
  name = sexp_intern(ctx, "SEXP_U16", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_U16));
  name = sexp_intern(ctx, "SEXP_S16", 8);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_S16));
  name = sexp_intern(ctx, "SEXP_U8", 7);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_U8));
  name = sexp_intern(ctx, "SEXP_S8", 7);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_S8));
  name = sexp_intern(ctx, "SEXP_U1", 7);
  sexp_env_define(ctx, env, name, tmp=sexp_make_integer(ctx, SEXP_U1));
  op = sexp_define_foreign(ctx, env, "c128vector-set!", 3, sexp_c128vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "c128vector-ref", 2, sexp_c128vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_OBJECT);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "c64vector-set!", 3, sexp_c64vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "c64vector-ref", 2, sexp_c64vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_OBJECT);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "f64vector-set!", 3, sexp_f64vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FLONUM);
  }
  op = sexp_define_foreign(ctx, env, "f64vector-ref", 2, sexp_f64vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FLONUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "f32vector-set!", 3, sexp_f32vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FLONUM);
  }
  op = sexp_define_foreign(ctx, env, "f32vector-ref", 2, sexp_f32vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FLONUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s64vector-set!", 3, sexp_s64vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s64vector-ref", 2, sexp_s64vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u64vector-set!", 3, sexp_u64vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u64vector-ref", 2, sexp_u64vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s32vector-set!", 3, sexp_s32vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s32vector-ref", 2, sexp_s32vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u32vector-set!", 3, sexp_u32vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u32vector-ref", 2, sexp_u32vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s16vector-set!", 3, sexp_s16vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s16vector-ref", 2, sexp_s16vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u16vector-set!", 3, sexp_u16vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u16vector-ref", 2, sexp_u16vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s8vector-set!", 3, sexp_s8vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s8vector-ref", 2, sexp_s8vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_UNIFORM_VECTOR);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u1vector-set!", 3, sexp_u1vector_set_x_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = SEXP_VOID;
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u1vector-ref", 2, sexp_u1vector_ref_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "c128vector?", 1, sexp_c128vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "c64vector?", 1, sexp_c64vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "f64vector?", 1, sexp_f64vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "f32vector?", 1, sexp_f32vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s64vector?", 1, sexp_s64vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u64vector?", 1, sexp_u64vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s32vector?", 1, sexp_s32vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u32vector?", 1, sexp_u32vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s16vector?", 1, sexp_s16vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u16vector?", 1, sexp_u16vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "s8vector?", 1, sexp_s8vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "u1vector?", 1, sexp_u1vector_p_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_BOOLEAN);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  op = sexp_define_foreign(ctx, env, "uvector-length", 1, sexp_uvector_length_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
  }
  sexp_gc_release3(ctx);
  return SEXP_VOID;
}

