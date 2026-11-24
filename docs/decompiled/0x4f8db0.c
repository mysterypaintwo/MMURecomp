// FUN_004f8db0 @ 0x4f8db0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f8db0(int param_1)

{
  float fVar1;
  int iVar2;
  float10 fVar3;
  undefined4 uVar4;
  
  iVar2 = FUN_006253a0();
  iVar2 = 10 - iVar2;
  uVar4 = 10;
  FUN_00403240(10,iVar2);
  FUN_00431250(uVar4,iVar2);
  fVar3 = (float10)FUN_0043e510();
  *(float *)(param_1 + 0x3478) = (float)(fVar3 + (float10)*(float *)(param_1 + 0x3478));
  fVar1 = *(float *)(param_1 + 0x3478);
  if ((!NAN(_DAT_0065e17c) && !NAN(fVar1)) && _DAT_0065e17c < fVar1 != (_DAT_0065e17c == fVar1)) {
    FUN_004405c0("SelectItem",param_1 + 4);
  }
  return;
}

