// FUN_0043c910 @ 0x43c910

undefined4 __thiscall FUN_0043c910(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  
  iVar1 = *(int *)(*(int *)(param_1 + 300) + 0xc);
  iVar2 = 0;
  if (0 < iVar1) {
    piVar4 = *(int **)(*(int *)(param_1 + 300) + 8);
    do {
      if (param_2 == *piVar4) {
        uVar3 = (**(code **)**(undefined4 **)
                              (*(int *)(*(int *)(param_1 + 300) + 8) + 4 + iVar2 * 0xc))(0,0);
        return uVar3;
      }
      iVar2 = iVar2 + 1;
      piVar4 = piVar4 + 3;
    } while (iVar2 < iVar1);
  }
  return 0xffffffff;
}

