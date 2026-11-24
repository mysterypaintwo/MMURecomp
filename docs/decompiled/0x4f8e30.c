// FUN_004f8e30 @ 0x4f8e30

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f8e30(int param_1)

{
  char cVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  float10 fVar5;
  undefined1 *puVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 local_38;
  undefined1 local_30 [24];
  float local_18;
  int local_14;
  char local_d;
  float local_c;
  undefined4 local_8;
  
  FUN_004f7400();
  cVar1 = FUN_00441d90();
  if (cVar1 != '\0') {
    local_8 = 0;
    cVar1 = FUN_004f6ff0(&local_8);
    if (cVar1 == '\0') {
      cVar1 = FUN_004f6f40();
      if (cVar1 == '\0') {
        cVar1 = FUN_004f6ef0();
        if (cVar1 == '\0') {
          cVar1 = FUN_004f6f90();
          if (cVar1 == '\0') {
            FUN_004026b0("PauseMenu",0,0);
            FUN_004405c0("SlideOut",param_1 + 4);
          }
          else {
            FUN_004026b0("PauseMenu",0,0);
            FUN_004405c0("SlideOut",param_1 + 4);
            FUN_00462a90();
            cVar1 = FUN_0041f990();
            if (cVar1 == '\0') {
              local_38 = _DAT_00698de8;
            }
            else {
              local_38 = _DAT_006981f0;
            }
            uVar9 = 0;
            puVar2 = (undefined4 *)FUN_0043bc20(0,0);
            uVar10 = puVar2[1];
            uVar8 = *puVar2;
            uVar7 = 0x35;
            uVar3 = FUN_0043bc10(local_38,0);
            puVar6 = local_30;
            FUN_00462a90(puVar6,uVar3,uVar7,uVar8,uVar10,uVar9);
            FUN_00401930();
            puVar2 = (undefined4 *)FUN_004014c0(puVar6,uVar3);
            FUN_005629b0(*puVar2,puVar2[1],uVar7,uVar8,uVar10,uVar9);
          }
        }
        else {
          local_d = '\0';
          local_c = _DAT_00698a88;
          for (local_14 = 0; local_14 < 0xd; local_14 = local_14 + 1) {
            fVar5 = (float10)FUN_00420e80(local_14);
            local_18 = (float)fVar5;
            if (local_18 < local_c) {
              local_d = '\x01';
              break;
            }
          }
          if (local_d == '\0') {
            FUN_004026b0("Wrong",0,0);
          }
          else {
            FUN_004405c0("WeaponEnergyFill",param_1 + 4);
          }
        }
      }
      else {
        iVar4 = FUN_00462a90();
        if ((*(uint *)(iVar4 + 0x150) < 0x1c) &&
           (iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4), iVar4 != 2)) {
          FUN_004405c0("EnergyFill",param_1 + 4);
        }
        else {
          FUN_004026b0("Wrong",0,0);
        }
      }
    }
    else {
      uVar10 = local_8;
      FUN_00462a90(local_8);
      FUN_00433090(uVar10);
      FUN_004026b0("PauseMenu",0,0);
      FUN_004405c0("SlideOut",param_1 + 4);
    }
  }
  uVar10 = 2;
  iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
  FUN_004f6dd0(param_1 + 0x20a0,iVar4,uVar10);
  uVar10 = 2;
  iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
  FUN_004f70e0(param_1 + 0x2330,iVar4,uVar10);
  uVar10 = 2;
  iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
  FUN_004f70e0(param_1 + 0x2480,iVar4,uVar10);
  uVar10 = 2;
  iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
  FUN_004f70e0(param_1 + 0x25d0,iVar4,uVar10);
  uVar10 = 2;
  iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
  FUN_004f70e0(param_1 + 0x2720,iVar4,uVar10);
  uVar10 = 3;
  iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
  FUN_004f70e0(param_1 + 0x2870,iVar4,uVar10);
  return;
}

