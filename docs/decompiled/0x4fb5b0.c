// FUN_004fb5b0 @ 0x4fb5b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004fb5b0(int param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  float10 fVar4;
  
  fVar4 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0xfc8) = (float)((float10)*(float *)(param_1 + 0xfc8) - fVar4);
  fVar1 = *(float *)(param_1 + 0xfc8);
  fVar2 = (float)_DAT_00697ce0;
  if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
    iVar3 = FUN_00402b30(*(undefined4 *)(param_1 + 0xfcc));
    *(undefined4 *)(param_1 + 0xfd0) = *(undefined4 *)(iVar3 + 0x1c);
  }
  return;
}

