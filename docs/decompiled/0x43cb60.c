// FUN_0043cb60 @ 0x43cb60

void __fastcall FUN_0043cb60(int param_1)

{
  int local_1c;
  int local_18;
  int local_14;
  float local_10;
  float local_c;
  
  if (*(int *)(param_1 + 0x78) != 0) {
    *(undefined1 *)(*(int *)(param_1 + 0x78) + 0x30) = *(undefined1 *)(param_1 + 0x69);
    FUN_004209b0(*(undefined1 *)(param_1 + 0x68));
    local_1c = 0;
    local_18 = 0;
    FUN_00425d50(&local_1c,&local_18);
    local_14 = FUN_006253a0();
    local_14 = local_14 - local_1c;
    local_10 = (float)local_14;
    local_14 = FUN_006253a0();
    local_14 = local_14 - local_18;
    local_c = (float)local_14;
    FUN_00401510(&local_10);
    FUN_004474d0(*(undefined4 *)(param_1 + 0x54));
  }
  return;
}

