// FUN_005a3bb0 @ 0x5a3bb0

void __fastcall FUN_005a3bb0(int param_1)

{
  char cVar1;
  
  FUN_004434d0();
  if (*(char *)(param_1 + 0xd0) == '\0') {
    FUN_00403220(0);
  }
  else {
    FUN_004434f0();
    cVar1 = FUN_00405360();
    if (cVar1 != '\0') {
      FUN_00422b80();
      cVar1 = FUN_0043a010();
      FUN_00403220(cVar1 == '\0');
      cVar1 = FUN_0043a010();
      FUN_00403220(cVar1 == '\0');
    }
  }
  FUN_0043cf60();
  return;
}

