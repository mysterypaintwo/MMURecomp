// FUN_004f5020 @ 0x4f5020

void __fastcall FUN_004f5020(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  int local_2c;
  undefined1 local_24 [18];
  char local_12;
  char local_11;
  int local_10;
  int local_c;
  int local_8;
  
  FUN_00431270();
  FUN_00401930();
  local_8 = FUN_006253a0();
  cVar1 = FUN_00441d90();
  if (cVar1 == '\0') {
    cVar1 = FUN_00441d90();
    local_2c = -(uint)(cVar1 != '\0');
  }
  else {
    local_2c = 1;
  }
  local_c = local_2c;
  if (local_2c == 0) {
    cVar1 = FUN_00441d90();
    if (cVar1 != '\0') {
      if (local_8 == 0xa0) {
        FUN_004405c0("SelectItem",param_1 + 4);
      }
      else {
        local_10 = 0;
        local_11 = FUN_004f3c40(&local_10);
        if (local_11 != '\0') {
          local_12 = '\0';
          switch(local_10) {
          case 0:
            local_12 = '\x01';
            FUN_00404a80();
            break;
          case 1:
            local_12 = '\x01';
            FUN_0061d9b0();
            break;
          case 2:
            local_12 = '\x01';
            FUN_0061da40();
            break;
          case 3:
            local_12 = '\x01';
            FUN_00404ac0();
            break;
          case 4:
            local_12 = '\x01';
            FUN_004332b0();
            break;
          case 5:
            local_12 = '\x01';
            FUN_00433270();
            break;
          case 6:
            local_12 = '\x01';
            FUN_004332f0();
            break;
          case 7:
            local_12 = '\x01';
            FUN_00433330();
          }
          if (local_12 != '\0') {
            FUN_0061d960(*(undefined4 *)(param_1 + 0x16e0 + local_10 * 4));
            FUN_004026b0("Purchase",0,0);
            FUN_004405c0("SelectItem",param_1 + 4);
          }
        }
      }
    }
  }
  else {
    uVar2 = FUN_0043bc20((-(uint)(local_8 != 0x40) & 0xffffffa0) + 0xa0,0xc0);
    puVar3 = local_24;
    FUN_00401930(puVar3,uVar2);
    uVar2 = FUN_004014c0(puVar3,uVar2);
    FUN_004026e0(uVar2);
    FUN_004026b0("Select",0,0);
  }
  return;
}

