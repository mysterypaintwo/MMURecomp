// FUN_0043cf70 @ 0x43cf70

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0043cf70(int param_1)

{
  float fVar1;
  uint uVar2;
  undefined4 uVar3;
  
  if (*(char *)(param_1 + 0xa6) != '\0') {
    if (_DAT_006c55e4 < (float)_DAT_006c1fd8) {
      _DAT_006c1fd8 = (double)_DAT_006c55e4;
    }
    fVar1 = _DAT_006c55e4;
    if ((float)_DAT_00698790 < _DAT_006c55e4) {
      fVar1 = _DAT_00698788;
    }
    fVar1 = *(float *)(param_1 + 0xa8) + fVar1;
    *(float *)(param_1 + 0xa8) = fVar1;
    if ((!NAN(_DAT_006989a8) && !NAN(fVar1)) && _DAT_006989a8 < fVar1 != (_DAT_006989a8 == fVar1)) {
      *(float *)(param_1 + 0xa8) = fVar1 - (float)_DAT_006989a0;
      *(bool *)(param_1 + 0xa7) = *(char *)(param_1 + 0xa7) == '\0';
    }
  }
  if (*(int *)(param_1 + 0x74) != 0) {
    FUN_00426a90(*(char *)(param_1 + 0x68) == '\0');
    FUN_004207a0(*(undefined1 *)(param_1 + 0x69));
    FUN_004311e0(*(undefined4 *)(param_1 + 0x6c));
    FUN_00401510(param_1 + 0x58);
    if ((*(char *)(param_1 + 0xa6) == '\0') || (*(char *)(param_1 + 0xa7) == '\0')) {
      FUN_00443540(*(undefined4 *)(param_1 + 0x74),param_1 + 0x110);
    }
    uVar2 = FUN_00507ec0();
    if ((*(uint *)(param_1 + 0x9c) & uVar2) != 0) {
      FUN_004434f0();
    }
    uVar3 = FUN_004207c0();
    *(undefined4 *)(param_1 + 0x78) = uVar3;
  }
  return;
}

