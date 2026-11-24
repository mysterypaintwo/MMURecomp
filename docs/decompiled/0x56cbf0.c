// FUN_0056cbf0 @ 0x56cbf0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056cbf0(int param_1)

{
  float *pfVar1;
  int iVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined1 local_c [8];
  
  FUN_00462a90();
  pfVar1 = (float *)FUN_00401930();
  *(float *)(param_1 + 0x380) = *pfVar1 - _DAT_006c6274;
  iVar2 = FUN_00401930();
  fVar4 = (float10)FUN_0043c800();
  *(float *)(param_1 + 900) = (float)(fVar4 + (float10)*(float *)(iVar2 + 4));
  uVar3 = FUN_00403d90(local_c,param_1 + 0x58);
  FUN_00401510(uVar3);
  FUN_0043bcd0();
  fVar4 = (float10)FUN_0043c800();
  FUN_004263f0((float)fVar4);
  fVar4 = (float10)FUN_0043c800();
  *(float *)(param_1 + 0x360) = (float)fVar4;
  return;
}

