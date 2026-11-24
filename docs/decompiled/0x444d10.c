// FUN_00444d10 @ 0x444d10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00444d10(int param_1)

{
  float fVar1;
  
  if (*(char *)(param_1 + 0x178) != '\0') {
    if (_DAT_006c55e4 < (float)_DAT_006c1fd8) {
      _DAT_006c1fd8 = (double)_DAT_006c55e4;
    }
    fVar1 = _DAT_006c55e4;
    if ((float)_DAT_00698790 < _DAT_006c55e4) {
      fVar1 = _DAT_00698788;
    }
    fVar1 = *(float *)(param_1 + 0x17c) - fVar1;
    *(float *)(param_1 + 0x17c) = fVar1;
    if (fVar1 <= 0.0) {
      *(undefined1 *)(param_1 + 0x178) = 0;
    }
  }
  return;
}

