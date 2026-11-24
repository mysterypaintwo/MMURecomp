// FUN_004f5900 @ 0x4f5900

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f5900(int param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  float *pfVar4;
  float10 fVar5;
  undefined4 uVar6;
  
  iVar3 = FUN_006253a0();
  iVar3 = 10 - iVar3;
  uVar6 = 10;
  FUN_00403240(10,iVar3);
  FUN_00431250(uVar6,iVar3);
  fVar5 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x1708) = (float)(fVar5 + (float10)*(float *)(param_1 + 0x1708));
  fVar1 = *(float *)(param_1 + 0x1708);
  if ((!NAN(_DAT_0065dae8) && !NAN(fVar1)) && _DAT_0065dae8 < fVar1 != (_DAT_0065dae8 == fVar1)) {
    pfVar4 = (float *)FUN_00431270();
    if ((float)_DAT_00698af8 < *pfVar4) {
      pfVar4 = (float *)FUN_00431270();
      fVar5 = (float10)FUN_0043e510();
      *pfVar4 = (float)((float10)*pfVar4 - fVar5 * (float10)_DAT_00698340);
    }
    pfVar4 = (float *)FUN_00431270();
    fVar1 = *pfVar4;
    fVar2 = (float)_DAT_00698af8;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      uVar6 = FUN_0043bc20(0xaa,0x97);
      FUN_004026e0(uVar6);
      uVar6 = 0;
      FUN_00403240(0);
      FUN_00403220(uVar6);
      FUN_00623fc0(param_1 + 0x1090);
      uVar6 = 1;
      FUN_00403240(1);
      FUN_00403220(uVar6);
    }
    pfVar4 = (float *)FUN_00431270();
    fVar1 = *pfVar4;
    fVar2 = (float)_DAT_00698af8;
    if (((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) &&
       ((float)_DAT_00698bb8 <= *(float *)(param_1 + 0x1708))) {
      FUN_004405c0("SelectItem",param_1 + 4);
    }
  }
  return;
}

