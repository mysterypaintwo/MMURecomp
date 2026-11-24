// FUN_0054baa0 @ 0x54baa0

void __fastcall FUN_0054baa0(int *param_1)

{
  int iVar1;
  
  FUN_004026b0("BossDoor",0,0);
  iVar1 = FUN_00420780();
  param_1[0x16] = (int)((float)(iVar1 + -1) + (float)param_1[0x16]);
  FUN_00507e90(0x10);
  (**(code **)(*param_1 + 0x10))(param_1 + 0x92);
  return;
}

