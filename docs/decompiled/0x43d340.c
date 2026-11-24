// FUN_0043d340 @ 0x43d340

void __thiscall FUN_0043d340(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  char cVar3;
  undefined4 uVar4;
  
  cVar3 = FUN_00461380();
  if (cVar3 == '\0') {
    *(undefined4 *)(param_1 + 0x74) = param_2;
    FUN_00403220(1);
    iVar1 = *(int *)(param_1 + 0x74);
    *(undefined4 *)(iVar1 + 0x6c) = 0;
    *(undefined4 *)(iVar1 + 0x68) = 0;
    if (*(int *)(*(int *)(iVar1 + 0x60) + 8) == 0) {
      *(undefined4 *)(iVar1 + 0x68) = 0;
    }
    iVar2 = *(int *)(*(int *)(iVar1 + 0x60) + 4);
    *(int *)(iVar1 + 100) = iVar2;
    iVar2 = *(int *)(iVar2 + 0x14);
    if (iVar2 != 0) {
      *(int *)(iVar1 + 0x2c) = iVar2;
    }
    uVar4 = FUN_004207c0();
    *(undefined4 *)(param_1 + 0x78) = uVar4;
    FUN_00422b80();
  }
  return;
}

