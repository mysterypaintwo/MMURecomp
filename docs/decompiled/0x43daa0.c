// FUN_0043daa0 @ 0x43daa0

void FUN_0043daa0(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int in_EAX;
  undefined4 local_20;
  undefined4 local_18;
  
  uVar1 = *(undefined4 *)(in_EAX + 0x18);
  FUN_00401090();
  FUN_004010b0(in_EAX);
  uVar2 = **(undefined4 **)(in_EAX + 0x18);
  FUN_00401090();
  FUN_004010b0(in_EAX);
  FUN_0043dcf0(local_18,uVar2,local_20,uVar1);
  operator_delete(*(void **)(in_EAX + 0x18));
  *(undefined4 *)(in_EAX + 0x18) = 0;
  *(undefined4 *)(in_EAX + 0x1c) = 0;
  return;
}

