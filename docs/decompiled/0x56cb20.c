// FUN_0056cb20 @ 0x56cb20

void __fastcall FUN_0056cb20(int param_1)

{
  undefined4 uVar1;
  float10 fVar2;
  undefined1 *puVar3;
  float fVar4;
  undefined1 local_1c [8];
  undefined1 local_14 [16];
  
  fVar2 = (float10)FUN_0043c800();
  fVar4 = (float)fVar2;
  fVar2 = (float10)FUN_0043c800(fVar4);
  uVar1 = FUN_0043bc10((float)fVar2,fVar4);
  puVar3 = local_14;
  FUN_00401930(puVar3,uVar1);
  uVar1 = FUN_004014c0(puVar3,uVar1);
  FUN_00401510(uVar1);
  uVar1 = FUN_00403d90(local_1c,param_1 + 0x58);
  FUN_00401510(uVar1);
  FUN_0043bcd0();
  fVar2 = (float10)FUN_0043c800();
  FUN_004263f0((float)fVar2);
  *(undefined4 *)(param_1 + 0x374) = 0;
  fVar2 = (float10)FUN_0043c800();
  *(float *)(param_1 + 0x360) = (float)fVar2;
  return;
}

