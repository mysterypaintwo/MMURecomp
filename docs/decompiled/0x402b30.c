// FUN_00402b30 @ 0x402b30

int __thiscall FUN_00402b30(int param_1,uint param_2)

{
  uint uVar1;
  
  uVar1 = FUN_00402b10();
  if (uVar1 <= param_2) {
    _invalid_parameter_noinfo();
  }
  return param_2 * 0x20 + *(int *)(param_1 + 0xc);
}

