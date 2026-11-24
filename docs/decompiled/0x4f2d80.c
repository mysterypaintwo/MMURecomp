// FUN_004f2d80 @ 0x4f2d80

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f2d80(int param_1)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  
  cVar1 = FUN_004308f0();
  if (cVar1 == '\0') {
    uVar3 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar3);
    FUN_00623fc0(param_1 + 0x58);
    uVar3 = FUN_0043bc20(0,0);
    FUN_004026e0(uVar3);
  }
  else {
    uVar3 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar3);
    FUN_00623fc0(param_1 + 0x220);
    uVar3 = FUN_0043bc20(0,0);
    FUN_004026e0(uVar3);
  }
  iVar2 = rand();
  if (_DAT_00698a90 <= (double)(iVar2 % 100) / _DAT_00698978) {
    FUN_004026b0("TitleScreenB",0,1);
  }
  else {
    FUN_004026b0("TitleScreenA",0,1);
  }
  FUN_00441b70(0);
  return;
}

