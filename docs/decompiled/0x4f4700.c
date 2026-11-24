// FUN_004f4700 @ 0x4f4700

void __fastcall FUN_004f4700(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = 0;
  FUN_00403240(0);
  FUN_00403220(uVar1);
  FUN_00441b70(1);
  *(undefined4 *)(param_1 + 0x1708) = 0;
  uVar1 = 0;
  FUN_00403240(0);
  FUN_00403220(uVar1);
  FUN_00623fc0(param_1 + 0x1128);
  uVar1 = 1;
  FUN_00403240(1);
  FUN_00403220(uVar1);
  Mix_HaltMusic();
  return;
}

