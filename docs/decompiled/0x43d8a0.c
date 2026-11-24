// FUN_0043d8a0 @ 0x43d8a0

float10 __fastcall FUN_0043d8a0(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar1 = *(int *)(param_1 + 300);
  if (iVar1 == 0) {
    return (float10)0.0;
  }
  iVar2 = 0;
  if (0 < *(int *)(iVar1 + 0x14)) {
    piVar3 = *(int **)(iVar1 + 0x10);
    do {
      if (*piVar3 == 0x6dc4ae65) {
        return (float10)(float)(*(int **)(iVar1 + 0x10))[iVar2 * 2 + 1];
      }
      iVar2 = iVar2 + 1;
      piVar3 = piVar3 + 2;
    } while (iVar2 < *(int *)(iVar1 + 0x14));
  }
  return (float10)0.0;
}

