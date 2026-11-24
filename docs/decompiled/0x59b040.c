// FUN_0059b040 @ 0x59b040

void __fastcall FUN_0059b040(int param_1)

{
  undefined4 uVar1;
  
  *(undefined4 *)(param_1 + 0x22c) = *(undefined4 *)(param_1 + 0x5c);
  FUN_00507e90(1);
  FUN_004405c0("Stand",param_1 + 4);
  uVar1 = FUN_00462a90();
  *(undefined4 *)(param_1 + 0x218) = uVar1;
  return;
}

