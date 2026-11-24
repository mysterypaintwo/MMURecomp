// FUN_00586020 @ 0x586020

void __fastcall FUN_00586020(int param_1)

{
  int iVar1;
  
  FUN_00401510(param_1 + 0x58);
  FUN_00462a90();
  iVar1 = FUN_00401930();
  *(bool *)(param_1 + 0x18c) = *(float *)(iVar1 + 4) < *(float *)(param_1 + 0x5c);
  return;
}

