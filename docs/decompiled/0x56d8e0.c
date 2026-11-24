// FUN_0056d8e0 @ 0x56d8e0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056d8e0(int param_1)

{
  float fVar1;
  float fVar2;
  float10 fVar3;
  
  fVar3 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x360) = (float)((float10)*(float *)(param_1 + 0x360) - fVar3);
  fVar1 = *(float *)(param_1 + 0x360);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    fVar3 = (float10)FUN_0043c800();
    FUN_0056d660((float)fVar3);
  }
  return;
}

