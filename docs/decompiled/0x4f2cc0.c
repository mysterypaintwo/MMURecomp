// FUN_004f2cc0 @ 0x4f2cc0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f2cc0(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  
  Mix_HaltMusic();
  FUN_00441b70(1);
  *(undefined4 *)(param_1 + 0x648) = _DAT_006983fc;
  cVar1 = FUN_004308f0();
  if (cVar1 == '\0') {
    uVar2 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar2);
    FUN_00623fc0(param_1 + 0x188);
    uVar2 = FUN_0043bc20(0,0);
    FUN_004026e0(uVar2);
  }
  else {
    uVar2 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar2);
    FUN_00623fc0(param_1 + 0x350);
    uVar2 = FUN_0043bc20(0,0);
    FUN_004026e0(uVar2);
  }
  return;
}

