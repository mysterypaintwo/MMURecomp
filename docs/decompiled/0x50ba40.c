// FUN_0050ba40 @ 0x50ba40

void __thiscall FUN_0050ba40(undefined4 *param_1,undefined4 param_2,char param_3)

{
  char cVar1;
  void *pvVar2;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00638e76;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  cVar1 = FUN_00461380(DAT_006c1e94 ^ (int)register0x00000010 - 4U);
  if (cVar1 == '\0') {
    FUN_00507fd0(1);
    *param_1 = 0x80;
  }
  pvVar2 = operator_new(0xd8);
  local_8 = 0;
  if (pvVar2 == (void *)0x0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_00621d10();
  }
  local_8 = 0xffffffff;
  param_1[5] = local_20;
  FUN_00621890(param_2);
  if (param_3 == '\0') {
    FUN_00430910();
  }
  ExceptionList = local_10;
  return;
}

