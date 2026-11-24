// FUN_004ff620 @ 0x4ff620

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004ff620(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = 1;
  FUN_00403240(1);
  FUN_004209d0(uVar1);
  FUN_004026b0("MMTeleportOUTOld",0,0);
  Mix_HaltMusic();
  FUN_00441b70(1);
  *(undefined4 *)(param_1 + 0xbac) = _DAT_00698ab0;
  FUN_00623fc0(param_1 + 0x220);
  uVar1 = FUN_0043bc20(0,0);
  FUN_004026e0(uVar1);
  FUN_00623fc0(param_1 + 0xb08);
  uVar1 = FUN_0043bc20(0x10,8);
  FUN_004026e0(uVar1);
  return;
}

