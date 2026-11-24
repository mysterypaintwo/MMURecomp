// FUN_00422bf0 @ 0x422bf0

void * __thiscall FUN_00422bf0(void *param_1,uint param_2)

{
  FUN_005a3e90();
  if ((param_2 & 1) != 0) {
    operator_delete(param_1);
  }
  return param_1;
}

