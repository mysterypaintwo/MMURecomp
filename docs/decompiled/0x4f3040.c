// FUN_004f3040 @ 0x4f3040

void __fastcall FUN_004f3040(int param_1)

{
  char cVar1;
  
  cVar1 = FUN_00441d90(param_1);
  if (cVar1 == '\0') {
    cVar1 = FUN_00443240();
    if (cVar1 == '\0') {
      FUN_00507e90(0x200);
    }
  }
  else {
    FUN_004026b0("Confirm",0,0);
    FUN_004405c0("Flash",param_1 + 4);
  }
  return;
}

