// FUN_004ffc30 @ 0x4ffc30

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004ffc30(int param_1)

{
  float fVar1;
  float fVar2;
  float10 fVar3;
  
  fVar3 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0xbac) = (float)((float10)*(float *)(param_1 + 0xbac) - fVar3);
  fVar1 = *(float *)(param_1 + 0xbac);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    FUN_004405c0("IntroSlideOut",param_1 + 4);
  }
  return;
}

