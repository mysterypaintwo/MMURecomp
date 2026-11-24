// FUN_0054f8f0 @ 0x54f8f0

void __fastcall FUN_0054f8f0(int param_1)

{
  int iVar1;
  float10 fVar2;
  
  iVar1 = FUN_00507ec0();
  if ((iVar1 != 2) && (iVar1 = FUN_00507ec0(), iVar1 != 4)) {
    fVar2 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0x150) = (float)(fVar2 + (float10)*(float *)(param_1 + 0x150));
    if (*(float *)(param_1 + 0x154) < *(float *)(param_1 + 0x150)) {
      FUN_0054f8a0();
    }
  }
  return;
}

