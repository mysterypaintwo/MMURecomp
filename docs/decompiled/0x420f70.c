// FUN_00420f70 @ 0x420f70

void * __thiscall FUN_00420f70(void *param_1,uint param_2)

{
  FUN_005b01f0();
  if ((param_2 & 1) != 0) {
    operator_delete(param_1);
  }
  return param_1;
}

