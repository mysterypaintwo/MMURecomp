// FUN_005a0930 @ 0x5a0930

void __fastcall FUN_005a0930(int param_1)

{
  float10 fVar1;
  
  fVar1 = (float10)FUN_0043c800();
  *(float *)(param_1 + 0x22c) = (float)fVar1;
  FUN_00507e90(1);
  FUN_004405c0(&DAT_0067cd2c,param_1 + 4);
  return;
}

