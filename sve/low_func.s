.align 16
.global get_zcr_el1
.global _get_zcr_el1
get_zcr_el1:
_get_zcr_el1:
#  mrs x0, zcr_el1
  .inst 0xd5381200
  ret

.align 16
.global get_fpcr
.global _get_fpcr
get_fpcr:
_get_fpcr:
  mrs x0, fpcr
  ret

.align 16
.global set_fpcr
.global _set_fpcr
set_fpcr:
_set_fpcr:
  msr fpcr, x0
  ret

.align 16
.global get_id_aa64isar0_el1
.global _get_id_aa64isar0_el1
get_id_aa64isar0_el1:
_get_id_aa64isar0_el1:
  mrs x0, id_aa64isar0_el1
  ret

.align 16
.global get_id_aa64pfr0_el1
.global _get_id_aa64pfr0_el1
get_id_aa64pfr0_el1:
_get_id_aa64pfr0_el1:
  mrs x0, id_aa64pfr0_el1
  ret
