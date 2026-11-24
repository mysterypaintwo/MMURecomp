// FUN_00503410 @ 0x503410

void __fastcall FUN_00503410(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  int local_24;
  undefined1 local_1c [16];
  int local_c;
  int local_8;
  
  FUN_00431270();
  FUN_00401930();
  local_8 = FUN_006253a0();
  cVar1 = FUN_00441d90();
  if (cVar1 == '\0') {
    cVar1 = FUN_00441d90();
    local_24 = -(uint)(cVar1 != '\0');
  }
  else {
    local_24 = 1;
  }
  local_c = local_24;
  if (local_24 == 0) {
    cVar1 = FUN_00441d90();
    if (cVar1 != '\0') {
      if (local_8 == 0x48) {
        FUN_0061dc80(0);
        FUN_00507e30(0);
        FUN_00507e10(0);
        FUN_004405c0("Flash",param_1 + 4);
      }
      else {
        FUN_004405c0("Select",param_1 + 4);
      }
    }
  }
  else {
    uVar2 = FUN_0043bc20((-(uint)(local_8 != 0x48) & 0xffffffb0) + 0x98,0x70);
    puVar3 = local_1c;
    FUN_00401930(puVar3,uVar2);
    uVar2 = FUN_004014c0(puVar3,uVar2);
    FUN_004026e0(uVar2);
    FUN_004026b0("Select",0,0);
  }
  return;
}

