.global xbyak_aarch64_get_fpcr
.global xbyak_aarch64_set_fpcr

xbyak_aarch64_get_fpcr:
  mrs x0, fpcr
  ret

xbyak_aarch64_set_fpcr:
  msr fpcr, x0
  ret


