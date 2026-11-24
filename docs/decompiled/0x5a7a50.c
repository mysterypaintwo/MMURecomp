// FUN_005a7a50 @ 0x5a7a50

void __fastcall FUN_005a7a50(int *param_1)

{
  char cVar1;
  undefined4 uVar2;
  char *local_18;
  char *local_14;
  
  if (*(char *)((int)param_1 + 0x1fd) != '\0') {
    local_14 = "LayingDown";
    goto LAB_005a7aa8;
  }
  if ((char)param_1[0xf7] == '\0') {
LAB_005a7a9b:
    local_18 = "stand";
  }
  else {
    cVar1 = FUN_0061daf0(3);
    if (cVar1 == '\0') goto LAB_005a7a9b;
    local_18 = "standglow";
  }
  local_14 = local_18;
LAB_005a7aa8:
  uVar2 = (**(code **)(*param_1 + 4))(local_14);
  (**(code **)(*param_1 + 0x10))(uVar2);
  *(undefined1 *)((int)param_1 + 0x1fd) = 0;
  if ((param_1[0x26] & 0x10U) == 0) {
    uVar2 = FUN_0043bc20(0,0);
    FUN_00401510(uVar2);
  }
  else {
    if (-(float)param_1[0x6e] <= (float)param_1[0x18]) {
      if ((float)param_1[0x6e] < (float)param_1[0x18]) {
        param_1[0x18] = param_1[0x6e];
      }
    }
    else {
      param_1[0x18] = (int)-(float)param_1[0x6e];
    }
    param_1[0x19] = 0;
  }
  *(undefined1 *)((int)param_1 + 0x3dd) = 0;
  *(undefined1 *)(param_1 + 0x76) = 0;
  return;
}

