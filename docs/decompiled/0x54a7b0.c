// FUN_0054a7b0 @ 0x54a7b0

void __fastcall FUN_0054a7b0(int *param_1)

{
  int iVar1;
  int local_c;
  
  FUN_004026b0("BossDoor",0,0);
  if ((char)param_1[0xd1] == '\0') {
    local_c = FUN_0041f400();
    local_c = local_c + -2;
  }
  else {
    iVar1 = FUN_0041f400();
    local_c = -(iVar1 + -2);
  }
  param_1[0x17] = (int)((float)local_c + (float)param_1[0x17]);
  FUN_00507e90(0x10);
  (**(code **)(*param_1 + 0x10))(param_1 + 0xb1);
  return;
}

