// FUN_00420860 @ 0x420860

void __fastcall FUN_00420860(int *param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  
  iVar1 = param_1[10];
  param_1[10] = param_1[9];
  if (param_1[9] != -1) {
    piVar3 = (int *)FUN_00433c10(param_1 + 9);
    iVar2 = *piVar3;
    if (*(int *)(iVar2 + 0x4c) == 0) {
      iVar4 = *param_1;
    }
    else {
      iVar4 = *param_1 + *(int *)(iVar2 + 0x48) +
              *(int *)(*(int *)(*param_1 + *(int *)(iVar2 + 0x48)) + *(int *)(iVar2 + 0x4c));
    }
    (**(code **)(iVar2 + 0x40))(iVar4 + *(int *)(iVar2 + 0x44));
  }
  param_1[9] = iVar1;
  return;
}

