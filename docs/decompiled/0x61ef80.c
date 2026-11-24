// FUN_0061ef80 @ 0x61ef80

void __fastcall FUN_0061ef80(undefined4 param_1)

{
  char cVar1;
  
  cVar1 = FUN_00403d40(param_1);
  if (cVar1 == '\0') {
    cVar1 = FUN_004027d0();
    if (cVar1 == '\0') {
      FUN_00434fe0(1);
      FUN_0061eb80();
    }
    else {
      cVar1 = FUN_004027d0();
      if (cVar1 == '\x01') {
        FUN_00434fe0(0);
        FUN_0061eb80();
      }
    }
  }
  else {
    FUN_00434fe0(0);
    FUN_0061eb80();
  }
  return;
}

