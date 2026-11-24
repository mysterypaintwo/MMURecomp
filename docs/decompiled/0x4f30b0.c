// FUN_004f30b0 @ 0x4f30b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f30b0(int param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  undefined4 uVar4;
  float10 fVar5;
  
  iVar3 = FUN_00431270();
  if (*(float *)(iVar3 + 4) < (float)_DAT_00698990) {
    iVar3 = FUN_00431270();
    fVar5 = (float10)FUN_0043e510();
    *(float *)(iVar3 + 4) = (float)(fVar5 * (float10)_DAT_006988f0 + (float10)*(float *)(iVar3 + 4))
    ;
  }
  iVar3 = FUN_00431270();
  if ((float)_DAT_00698990 <= *(float *)(iVar3 + 4)) {
    FUN_00623fc0(param_1 + 0x518);
    uVar4 = FUN_0043bc20(0x14,0x10);
    FUN_004026e0(uVar4);
    uVar4 = FUN_0043bc20(0x14,0x10);
    FUN_004026e0(uVar4);
  }
  iVar3 = FUN_00431270();
  if ((float)_DAT_00698c00 < *(float *)(iVar3 + 4)) {
    iVar3 = FUN_00431270();
    if (*(float *)(iVar3 + 4) == (float)_DAT_00698990) {
      iVar3 = FUN_00431270();
      fVar5 = (float10)FUN_0043e510();
      *(float *)(iVar3 + 4) =
           (float)((float10)*(float *)(iVar3 + 4) - fVar5 * (float10)_DAT_00698898);
    }
  }
  iVar3 = FUN_00431270();
  fVar1 = *(float *)(iVar3 + 4);
  fVar2 = (float)_DAT_00698c00;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    FUN_00623fc0(param_1 + 0x5b0);
    uVar4 = FUN_0043bc20(0x19,0x3a);
    FUN_004026e0(uVar4);
    uVar4 = FUN_0043bc20(0x19,0x3a);
    FUN_004026e0(uVar4);
  }
  iVar3 = FUN_00431270();
  if (*(float *)(iVar3 + 4) == (float)_DAT_00698990) {
    iVar3 = FUN_00431270();
    if (*(float *)(iVar3 + 4) == (float)_DAT_00698c00) {
      FUN_004405c0(&DAT_0065da6c,param_1 + 4);
    }
  }
  return;
}

