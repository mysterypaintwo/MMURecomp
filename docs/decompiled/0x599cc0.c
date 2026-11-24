// FUN_00599cc0 @ 0x599cc0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00599cc0(int param_1)

{
  FUN_00507e90(1);
  *(undefined4 *)(param_1 + 0x248) = _DAT_00697cf0;
  *(undefined4 *)(param_1 + 0x24c) = 0x3f800000;
  FUN_0043bcd0();
  FUN_004405c0("AttackOneJump",param_1 + 4);
  return;
}

