// FUN_004fb610 @ 0x4fb610

void __fastcall FUN_004fb610(int param_1)

{
  undefined4 uVar1;
  
  *(undefined4 *)(param_1 + 0x100c) = 0xffffffff;
  FUN_004650c0();
  uVar1 = 1;
  FUN_00403240(1);
  FUN_00403220(uVar1);
  *(undefined1 *)(param_1 + 0x1034) = 0;
  return;
}

