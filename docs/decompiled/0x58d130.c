// FUN_0058d130 @ 0x58d130

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0058d130(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = rand();
  param_1[99] = (int)((float)param_1[0x65] - (float)iVar1 / (float)_DAT_00698418);
  uVar2 = (**(code **)(*param_1 + 4))("Drill");
  (**(code **)(*param_1 + 0x10))(uVar2);
  return;
}

