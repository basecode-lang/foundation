/* Automatically generated by chibi-ffi; version: 0.4 */

#include <chibi/eval.h>

#include <util.h>
/*
types: (winsize termios)
enums: ()
*/

sexp sexp_login_tty_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg0) {
  sexp res;
  if (! (sexp_filenop(arg0) || sexp_fixnump(arg0)))
    return sexp_type_exception(ctx, self, SEXP_FILENO, arg0);
  res = sexp_make_integer(ctx, login_tty((sexp_filenop(arg0) ? sexp_fileno_fd(arg0) : sexp_unbox_fixnum(arg0))));
  return res;
}

sexp sexp_forkpty_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg2, sexp arg3) {
  int tmp0;
  char tmp1[256];
  sexp_gc_var3(res, res0, res1);
  if (! ((sexp_pointerp(arg2) && (sexp_pointer_tag(arg2) == sexp_unbox_fixnum(sexp_opcode_arg3_type(self)))) || sexp_not(arg2)))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_opcode_arg3_type(self)), arg2);
  if (! ((sexp_pointerp(arg3) && (sexp_pointer_tag(arg3) == sexp_unbox_fixnum(sexp_vector_ref(sexp_opcode_argn_type(self), SEXP_ZERO)))) || sexp_not(arg3)))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_vector_ref(sexp_opcode_argn_type(self), SEXP_ZERO)), arg3);
  sexp_gc_preserve3(ctx, res, res0, res1);
  res = sexp_make_unsigned_integer(ctx, forkpty(&tmp0, tmp1, (struct termios*)sexp_cpointer_maybe_null_value(arg2), (struct winsize*)sexp_cpointer_maybe_null_value(arg3)));
  res0 = sexp_make_fileno(ctx, sexp_make_fixnum(tmp0), SEXP_FALSE);
  res1 = sexp_c_string(ctx, tmp1, -1);
  res = sexp_cons(ctx, res, SEXP_NULL);
  sexp_push(ctx, res, sexp_car(res));
  sexp_cadr(res) = res1;
  sexp_push(ctx, res, sexp_car(res));
  sexp_cadr(res) = res0;
  sexp_gc_release3(ctx);
  return res;
}

sexp sexp_openpty_stub (sexp ctx, sexp self, sexp_sint_t n, sexp arg3, sexp arg4) {
  int err = 0;
  int tmp0;
  int tmp1;
  char tmp2[256];
  sexp_gc_var4(res, res0, res1, res2);
  if (! ((sexp_pointerp(arg3) && (sexp_pointer_tag(arg3) == sexp_unbox_fixnum(sexp_vector_ref(sexp_opcode_argn_type(self), SEXP_ZERO)))) || sexp_not(arg3)))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_vector_ref(sexp_opcode_argn_type(self), SEXP_ZERO)), arg3);
  if (! ((sexp_pointerp(arg4) && (sexp_pointer_tag(arg4) == sexp_unbox_fixnum(sexp_vector_ref(sexp_opcode_argn_type(self), SEXP_ONE)))) || sexp_not(arg4)))
    return sexp_type_exception(ctx, self, sexp_unbox_fixnum(sexp_vector_ref(sexp_opcode_argn_type(self), SEXP_ONE)), arg4);
  sexp_gc_preserve4(ctx, res, res0, res1, res2);
  err = openpty(&tmp0, &tmp1, tmp2, (struct termios*)sexp_cpointer_maybe_null_value(arg3), (struct winsize*)sexp_cpointer_maybe_null_value(arg4));
  if (err) {
  res = SEXP_FALSE;
  } else {
  res0 = sexp_make_fileno(ctx, sexp_make_fixnum(tmp0), SEXP_FALSE);
  res1 = sexp_make_fileno(ctx, sexp_make_fixnum(tmp1), SEXP_FALSE);
  res2 = sexp_c_string(ctx, tmp2, -1);
  res = SEXP_NULL;
  sexp_push(ctx, res, res2);
  sexp_push(ctx, res, res1);
  sexp_push(ctx, res, res0);
  }
  sexp_gc_release4(ctx);
  return res;
}


sexp sexp_init_library (sexp ctx, sexp self, sexp_sint_t n, sexp env, const char* version, const sexp_abi_identifier_t abi) {
  sexp sexp_winsize_type_obj;
  sexp sexp_termios_type_obj;
  sexp_gc_var3(name, tmp, op);
  if (!(sexp_version_compatible(ctx, version, sexp_version)
        && sexp_abi_compatible(ctx, abi, SEXP_ABI_IDENTIFIER)))
    return SEXP_ABI_ERROR;
  sexp_gc_preserve3(ctx, name, tmp, op);
  name = sexp_intern(ctx, "winsize", -1);
  sexp_winsize_type_obj = sexp_env_ref(ctx, env, name, SEXP_FALSE);
  if (sexp_not(sexp_winsize_type_obj)) {
    sexp_warn(ctx, "couldn't import declared type: ", name);
  }
  name = sexp_intern(ctx, "termios", -1);
  sexp_termios_type_obj = sexp_env_ref(ctx, env, name, SEXP_FALSE);
  if (sexp_not(sexp_termios_type_obj)) {
    sexp_warn(ctx, "couldn't import declared type: ", name);
  }
  op = sexp_define_foreign(ctx, env, "login-tty", 1, sexp_login_tty_stub);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FILENO);
  }
  op = sexp_define_foreign_opt(ctx, env, "forkpty", 2, sexp_forkpty_stub, SEXP_FALSE);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_FIXNUM);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FILENO);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_CHAR);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(sexp_type_tag(sexp_termios_type_obj));
    sexp_opcode_argn_type(op) = sexp_make_vector(ctx, SEXP_ONE, sexp_make_fixnum(SEXP_OBJECT));
    sexp_vector_set(sexp_opcode_argn_type(op), SEXP_ZERO, sexp_make_fixnum(sexp_type_tag(sexp_winsize_type_obj)));
  }
  op = sexp_define_foreign_opt(ctx, env, "openpty", 2, sexp_openpty_stub, SEXP_FALSE);
  if (sexp_opcodep(op)) {
    sexp_opcode_return_type(op) = sexp_make_fixnum(SEXP_OBJECT);
    sexp_opcode_arg1_type(op) = sexp_make_fixnum(SEXP_FILENO);
    sexp_opcode_arg2_type(op) = sexp_make_fixnum(SEXP_FILENO);
    sexp_opcode_arg3_type(op) = sexp_make_fixnum(SEXP_CHAR);
    sexp_opcode_argn_type(op) = sexp_make_vector(ctx, SEXP_TWO, sexp_make_fixnum(SEXP_OBJECT));
    sexp_vector_set(sexp_opcode_argn_type(op), SEXP_ZERO, sexp_make_fixnum(sexp_type_tag(sexp_termios_type_obj)));
    sexp_vector_set(sexp_opcode_argn_type(op), SEXP_ONE, sexp_make_fixnum(sexp_type_tag(sexp_winsize_type_obj)));
  }
  sexp_gc_release3(ctx);
  return SEXP_VOID;
}

