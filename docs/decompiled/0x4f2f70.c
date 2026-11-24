// FUN_004f2f70 @ 0x4f2f70

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f2f70(int param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  float10 fVar4;
  undefined4 uVar5;
  
  fVar4 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x648) = (float)((float10)*(float *)(param_1 + 0x648) - fVar4);
  uVar5 = 0;
  FUN_00403240(0);
  FUN_00403220(uVar5);
  uVar5 = 0;
  FUN_00403240(0);
  FUN_00403220(uVar5);
  iVar3 = FUN_00431270();
  fVar4 = (float10)FUN_0043e510();
  *(float *)(iVar3 + 4) = (float)((float10)*(float *)(iVar3 + 4) - fVar4 * (float10)_DAT_00698bf8);
  iVar3 = FUN_00431270();
  fVar4 = (float10)FUN_0043e510();
  *(float *)(iVar3 + 4) = (float)(fVar4 * (float10)_DAT_00698bf8 + (float10)*(float *)(iVar3 + 4));
  fVar1 = *(float *)(param_1 + 0x648);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    FUN_004405c0("IntroFadeOut",param_1 + 4);
  }
  return;
}

