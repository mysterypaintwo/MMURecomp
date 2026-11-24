// FUN_00560c80 @ 0x560c80

void __fastcall FUN_00560c80(int param_1)

{
  undefined4 uVar1;
  uint uVar2;
  uint local_2c;
  undefined1 local_18 [8];
  undefined1 local_10 [8];
  uint local_8;
  
  if (*(int *)(param_1 + 0x74) != 0) {
    FUN_00426a90(*(char *)(param_1 + 0x68) == '\0');
    local_8 = 0;
    while( true ) {
      local_2c = (uint)(longlong)ROUND(*(float *)(param_1 + 0x170));
      if (local_2c <= local_8) break;
      uVar1 = FUN_00401540(local_10,(float)local_8);
      FUN_004014c0(local_18,uVar1);
      FUN_004434d0();
      FUN_00443540(*(undefined4 *)(param_1 + 0x74),0);
      local_8 = local_8 + 1;
    }
    uVar2 = FUN_00507ec0();
    if ((uVar2 & *(uint *)(param_1 + 0x9c)) != 0) {
      FUN_004434f0();
    }
    uVar1 = FUN_004207c0();
    *(undefined4 *)(param_1 + 0x78) = uVar1;
  }
  return;
}

