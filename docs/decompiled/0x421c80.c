// FUN_00421c80 @ 0x421c80

void * __thiscall FUN_00421c80(void *param_1,uint param_2)

{
  FUN_00401980();
  if ((param_2 & 1) != 0) {
    operator_delete(param_1);
  }
  return param_1;
}

