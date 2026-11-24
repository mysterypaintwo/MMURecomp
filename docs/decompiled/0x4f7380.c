// FUN_004f7380 @ 0x4f7380

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f7380(int param_1)

{
  float fVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined4 uVar4;
  
  uVar2 = FUN_006253a0();
  uVar4 = 10;
  FUN_00403240(10,uVar2);
  FUN_00431250(uVar4,uVar2);
  fVar3 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x3478) = (float)(fVar3 + (float10)*(float *)(param_1 + 0x3478));
  fVar1 = *(float *)(param_1 + 0x3478);
  if ((!NAN(_DAT_0065e17c) && !NAN(fVar1)) && _DAT_0065e17c < fVar1 != (_DAT_0065e17c == fVar1)) {
    FUN_00507f00(0);
    FUN_00441b70(0);
  }
  return;
}

