// FUN_004045a0 @ 0x4045c0

void * __thiscall FUN_004045a0(void *param_1,uint param_2)

{
  FUN_005e7340();
  if ((param_2 & 1) != 0) {
    operator_delete(param_1);
  }
  return param_1;
}

