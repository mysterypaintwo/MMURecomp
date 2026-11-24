// FUN_0056ccf0 @ 0x56ccf0

void __fastcall FUN_0056ccf0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined1 local_14 [16];
  
  fVar3 = (float10)FUN_0043c800();
  *(float *)(param_1 + 0x360) = (float)fVar3;
  iVar1 = FUN_00401930();
  fVar3 = (float10)FUN_0043c800();
  uVar2 = FUN_0043bc10(*(undefined4 *)(param_1 + 0x58),
                       (float)(fVar3 + (float10)*(float *)(iVar1 + 4)));
  FUN_00401510(uVar2);
  uVar2 = FUN_00403d90(local_14,param_1 + 0x58);
  FUN_00401510(uVar2);
  FUN_0043bcd0();
  fVar3 = (float10)FUN_0043c800();
  FUN_004263f0((float)fVar3);
  return;
}

