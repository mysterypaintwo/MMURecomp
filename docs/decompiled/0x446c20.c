// FUN_00446c20 @ 0x446c20

int FUN_00446c20(int param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_00643a08;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00447270(param_1);
  local_4 = 0;
  FUN_0043e470(*(undefined4 *)(*(int *)(param_1 + 0x18) + 4));
  *(int *)(*(int *)(param_1 + 0x18) + 4) = *(int *)(param_1 + 0x18);
  *(undefined4 *)(param_1 + 0x1c) = 0;
  *(undefined4 *)*(undefined4 *)(param_1 + 0x18) = *(undefined4 *)(param_1 + 0x18);
  *(int *)(*(int *)(param_1 + 0x18) + 8) = *(int *)(param_1 + 0x18);
  *(undefined1 *)(param_1 + 0x20) = 0;
  *(undefined1 *)(param_1 + 0x21) = 1;
  ExceptionList = local_c;
  return param_1;
}

