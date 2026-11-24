// FUN_0043f0f0 @ 0x43f0f0

void FUN_0043f0f0(undefined4 param_1)

{
  int iVar1;
  int in_EAX;
  int iVar2;
  
  iVar1 = *(int *)(in_EAX + 0x14);
  FUN_00492540();
  FUN_004010b0(in_EAX);
  iVar2 = FUN_0043db30(iVar1,*(undefined4 *)(iVar1 + 4),param_1);
  FUN_0043fa70();
  *(int *)(iVar1 + 4) = iVar2;
  **(int **)(iVar2 + 4) = iVar2;
  return;
}

