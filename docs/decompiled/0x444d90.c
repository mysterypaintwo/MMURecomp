// FUN_00444d90 @ 0x444d90

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_00444d90(int param_1)

{
  float *pfVar1;
  float fVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  float10 fVar6;
  int local_10;
  
  pfVar1 = (float *)(param_1 + 0x58);
  FUN_00401510(pfVar1);
  if (*(int *)(param_1 + 0x168) == 0) {
    if (*(int *)(param_1 + 0x154) != 0) {
      FUN_00401930();
      uVar3 = FUN_00444b20();
      FUN_00401510(uVar3);
    }
  }
  else {
    switch(*(int *)(param_1 + 0x168)) {
    case 1:
    case 2:
      fVar6 = (float10)FUN_0043e510();
      fVar2 = (float)(fVar6 * (float10)_DAT_006987d0);
      if (fVar2 <= *(float *)(param_1 + 0x16c)) {
        *(float *)(param_1 + 0x16c) = *(float *)(param_1 + 0x16c) - fVar2;
        *pfVar1 = fVar2 * (float)(int)((uint)(*(int *)(param_1 + 0x168) != 2) * 2 + -1) + *pfVar1;
      }
      else {
        *pfVar1 = (float)(int)((uint)(*(int *)(param_1 + 0x168) != 2) * 2 + -1) *
                  *(float *)(param_1 + 0x16c) + *pfVar1;
        FUN_00444af0();
      }
      if (*(int *)(param_1 + 0x168) == 0) {
        iVar4 = FUN_006253a0();
        *pfVar1 = (float)iVar4;
      }
      break;
    case 3:
    case 4:
    case 5:
    case 6:
      fVar6 = (float10)FUN_0043e510();
      fVar2 = (float)(fVar6 * (float10)_DAT_00698448);
      iVar4 = *(int *)(param_1 + 0x168);
      if (fVar2 <= *(float *)(param_1 + 0x16c)) {
        *(float *)(param_1 + 0x16c) = *(float *)(param_1 + 0x16c) - fVar2;
        if ((iVar4 == 3) || (local_10 = 1, iVar4 == 5)) {
          local_10 = -1;
        }
        *(float *)(param_1 + 0x5c) = fVar2 * (float)local_10 + *(float *)(param_1 + 0x5c);
      }
      else {
        if ((iVar4 == 3) || (local_10 = 1, iVar4 == 5)) {
          local_10 = -1;
        }
        *(float *)(param_1 + 0x5c) =
             (float)local_10 * *(float *)(param_1 + 0x16c) + *(float *)(param_1 + 0x5c);
        FUN_00444af0();
      }
      if (*(int *)(param_1 + 0x168) == 0) {
        iVar4 = FUN_006253a0();
        *(float *)(param_1 + 0x5c) = (float)iVar4;
      }
    }
  }
  iVar4 = FUN_00401510(pfVar1);
  if ((*(char *)(param_1 + 0x178) != '\0') && (iVar4 = FUN_00507ec0(), iVar4 == 1)) {
    iVar5 = rand();
    iVar4 = iVar5 / 5;
    *(float *)(param_1 + 0x184) = (float)(iVar5 % 5 + -2) + *(float *)(param_1 + 0x184);
  }
  return iVar4;
}

