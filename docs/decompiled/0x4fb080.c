// FUN_004fb080 @ 0x4fb080

void __fastcall FUN_004fb080(int param_1)

{
  if (*(int *)(param_1 + 0xfd0) == 0x41) {
    FUN_00402790(*(undefined4 *)(param_1 + 0x1010));
  }
  else if (*(int *)(param_1 + 0xfd0) != 0) {
    FUN_00402790(*(undefined4 *)(param_1 + 0xfd0));
  }
  *(undefined4 *)(param_1 + 0xfd0) = 0;
  return;
}

