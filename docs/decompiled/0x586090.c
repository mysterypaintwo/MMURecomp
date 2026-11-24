// FUN_00586090 @ 0x586090

void __fastcall FUN_00586090(int param_1)

{
  float *pfVar1;
  
  FUN_00401510(param_1 + 0x58);
  FUN_00462a90();
  pfVar1 = (float *)FUN_00401930();
  *(bool *)(param_1 + 0x18c) = *(float *)(param_1 + 0x58) < *pfVar1;
  return;
}

