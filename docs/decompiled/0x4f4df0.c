// FUN_004f4df0 @ 0x4f4df0

void __fastcall FUN_004f4df0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  FUN_00623fc0(param_1 + 0x58);
  uVar1 = FUN_00401930();
  FUN_004026e0(uVar1);
  FUN_00441b70(1);
  *(undefined4 *)(param_1 + 0x1708) = 0;
  FUN_004026b0("MenuThemeA",1,1);
  iVar2 = rand();
  if (iVar2 % 100 < 0x61) {
    FUN_00623fc0(param_1 + 0x1258);
  }
  else {
    FUN_00623fc0(param_1 + 0x1420);
  }
  FUN_00623fc0(param_1 + 0x12f0);
  FUN_00623fc0(param_1 + 5000);
  FUN_00623fc0(param_1 + 0xff8);
  return;
}

