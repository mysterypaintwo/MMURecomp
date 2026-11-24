// FUN_004f2e80 @ 0x4f2e80

void __fastcall FUN_004f2e80(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  
  FUN_00507f40(param_1);
  FUN_00441b70(1);
  Mix_HaltMusic();
  cVar1 = FUN_004308f0();
  if (cVar1 == '\0') {
    FUN_00623fc0(param_1 + 0xf0);
    uVar2 = FUN_0043bc20(0,0);
    FUN_004026e0(uVar2);
  }
  else {
    FUN_00623fc0(param_1 + 0x2b8);
    uVar2 = FUN_0043bc20(0,0);
    FUN_004026e0(uVar2);
  }
  FUN_00623fc0(param_1 + 1000);
  FUN_00623fc0(param_1 + 0x480);
  uVar2 = FUN_0043bc20(0x14,0xffffffc0);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x19,0xe1);
  FUN_004026e0(uVar2);
  return;
}

