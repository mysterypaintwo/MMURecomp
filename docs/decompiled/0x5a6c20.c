// FUN_005a6c20 @ 0x5a6c20

undefined4 __thiscall FUN_005a6c20(undefined4 param_1,int *param_2)

{
  char cVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 local_14 [8];
  undefined1 local_c [8];
  
  if ((param_2 == (int *)0x0) || (cVar1 = FUN_00421c50("InvinsibilityDuration"), cVar1 == '\0')) {
    uVar2 = FUN_004015d0(param_1);
    return uVar2;
  }
  uVar2 = FUN_0046a130(local_14);
  uVar3 = (**(code **)(*param_2 + 0x24))(uVar2);
  FUN_00421eb0(local_c,uVar3);
  cVar1 = FUN_0046bcb0(uVar2);
  if (cVar1 == '\0') {
    return 0;
  }
  return 1;
}

