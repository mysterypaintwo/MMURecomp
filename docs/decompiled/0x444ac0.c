// FUN_00444ac0 @ 0x444ac0

undefined4 * __thiscall FUN_00444ac0(undefined4 *param_1,byte param_2)

{
  *param_1 = Camera::vftable;
  FUN_0043c320();
  if ((param_2 & 1) != 0) {
    operator_delete(param_1);
  }
  return param_1;
}

