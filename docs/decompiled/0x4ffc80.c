// FUN_004ffc80 @ 0x4ffc80

void __fastcall FUN_004ffc80(int param_1)

{
  char cVar1;
  int *piVar2;
  int iVar3;
  int local_14;
  int local_10 [3];
  
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    local_10[2] = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
    local_10[2] = local_10[2] + 1;
    local_10[1] = 5;
    piVar2 = std::_Min_value<int>(local_10 + 1,local_10 + 2);
    FUN_004309b0(*piVar2);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    local_10[0] = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
    local_10[0] = local_10[0] + -1;
    local_14 = 0;
    piVar2 = std::_Min_value<int>(&local_14,local_10);
    FUN_004309b0(*piVar2);
  }
  FUN_004ff460();
  FUN_004ff070();
  if ((*(char *)(param_1 + 0xbb4) == '\0') && (cVar1 = FUN_00443240(), cVar1 == '\0')) {
    FUN_004026b0("StageSelectBLoop",1,1);
    *(undefined1 *)(param_1 + 0xbb4) = 1;
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    if ((*(int *)(param_1 + 0xba0) == 0) && (*(int *)(param_1 + 0xba4) == 0)) {
      *(undefined4 *)(param_1 + 0xbb0) = 8;
    }
    else if ((*(int *)(param_1 + 0xba0) == 1) && (*(int *)(param_1 + 0xba4) == 0)) {
      *(undefined4 *)(param_1 + 0xbb0) = 1;
    }
    else if ((*(int *)(param_1 + 0xba0) == 2) && (*(int *)(param_1 + 0xba4) == 0)) {
      *(undefined4 *)(param_1 + 0xbb0) = 3;
    }
    else if ((*(int *)(param_1 + 0xba0) == 0) && (*(int *)(param_1 + 0xba4) == 1)) {
      *(undefined4 *)(param_1 + 0xbb0) = 7;
    }
    else if ((*(int *)(param_1 + 0xba0) == 1) && (*(int *)(param_1 + 0xba4) == 1)) {
      if (*(char *)(param_1 + 0xba8) == '\0') {
        iVar3 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
        if (iVar3 == 1) {
          *(undefined4 *)(param_1 + 0xbb0) = 10;
        }
        else {
          iVar3 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
          if (iVar3 == 2) {
            *(undefined4 *)(param_1 + 0xbb0) = 0xb;
          }
          else {
            iVar3 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
            if (iVar3 == 3) {
              *(undefined4 *)(param_1 + 0xbb0) = 0xc;
            }
            else {
              iVar3 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
              if (iVar3 == 4) {
                *(undefined4 *)(param_1 + 0xbb0) = 0xd;
              }
              else {
                iVar3 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
                if (iVar3 == 5) {
                  *(undefined4 *)(param_1 + 0xbb0) = 0xe;
                }
              }
            }
          }
        }
      }
      else {
        *(undefined4 *)(param_1 + 0xbb0) = 9;
      }
    }
    else if ((*(int *)(param_1 + 0xba0) == 2) && (*(int *)(param_1 + 0xba4) == 1)) {
      *(undefined4 *)(param_1 + 0xbb0) = 6;
    }
    else if ((*(int *)(param_1 + 0xba0) == 0) && (*(int *)(param_1 + 0xba4) == 2)) {
      *(undefined4 *)(param_1 + 0xbb0) = 4;
    }
    else if ((*(int *)(param_1 + 0xba0) == 1) && (*(int *)(param_1 + 0xba4) == 2)) {
      *(undefined4 *)(param_1 + 0xbb0) = 2;
    }
    else if ((*(int *)(param_1 + 0xba0) == 2) && (*(int *)(param_1 + 0xba4) == 2)) {
      *(undefined4 *)(param_1 + 0xbb0) = 5;
    }
    if (*(int *)(param_1 + 0xbb0) == 0) {
      return;
    }
    FUN_004405c0("Flash",param_1 + 4);
  }
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    FUN_004026b0("Confirm",0,0);
    FUN_004026b0("MenuThemeB",1,1);
    FUN_00402790(3);
  }
  return;
}

