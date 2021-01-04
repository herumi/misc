.global xbyak_aarch64_get_fpcr
.global _xbyak_aarch64_get_fpcr
.global xbyak_aarch64_set_fpcr
.global _xbyak_aarch64_set_fpcr

.align 16
xbyak_aarch64_get_fpcr:
_xbyak_aarch64_get_fpcr:
  mrs x0, fpcr
  ret

.align 16
xbyak_aarch64_set_fpcr:
_xbyak_aarch64_set_fpcr:
  msr fpcr, x0
  ret


