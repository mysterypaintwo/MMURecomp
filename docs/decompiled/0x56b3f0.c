// FUN_0056b3f0 @ 0x56b3f0

void __fastcall FUN_0056b3f0(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  
  if ((*(char *)(param_1 + 0x35c) != '\0') && (*(int *)(param_1 + 0x150) == 0)) {
    cVar1 = FUN_004ad6a0();
    if (cVar1 != '\0') {
      FUN_004047a0(1);
      uVar2 = FUN_0041efa0();
      FUN_0043e590(uVar2);
      cVar1 = FUN_00403d40();
      if (cVar1 == '\0') {
        FUN_00507e90(0x40);
        Mix_HaltMusic();
        return;
      }
      FUN_0061eea0();
      FUN_00507e90(0x100);
      Mix_HaltMusic();
      return;
    }
    cVar1 = FUN_00461870();
    if (cVar1 != '\0') {
      FUN_006235b0();
    }
  }
  FUN_00575670();
  return;
}

