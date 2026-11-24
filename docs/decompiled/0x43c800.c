// FUN_0043c800 @ 0x43c800

float10 FUN_0043c800(void)

{
  int iVar1;
  int in_EAX;
  int iVar2;
  int *piVar3;
  int unaff_EDI;
  
  iVar1 = *(int *)(in_EAX + 300);
  if (iVar1 == 0) {
    return (float10)0;
  }
  iVar2 = 0;
  if (0 < *(int *)(iVar1 + 0x14)) {
    piVar3 = *(int **)(iVar1 + 0x10);
    do {
      if (unaff_EDI == *piVar3) {
        return (float10)(float)(*(int **)(iVar1 + 0x10))[iVar2 * 2 + 1];
      }
      iVar2 = iVar2 + 1;
      piVar3 = piVar3 + 2;
    } while (iVar2 < *(int *)(iVar1 + 0x14));
  }
  return (float10)0;
}

