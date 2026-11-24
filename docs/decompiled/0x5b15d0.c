// FUN_005b15d0 @ 0x5b15d0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005b15d0(int *param_1)

{
  float fVar1;
  bool bVar2;
  float fVar3;
  char cVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar5;
  char *pcVar6;
  int iVar7;
  undefined4 *puVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  int *piVar11;
  float10 fVar12;
  undefined4 uVar13;
  char *pcVar14;
  char local_50;
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [11];
  char local_25;
  undefined4 local_24;
  undefined4 local_20;
  char local_19;
  int local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  char local_5;
  
  pcVar14 = "Death";
  pbVar5 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
  pcVar6 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar5);
  iVar7 = strcmp(pcVar6,pcVar14);
  if (iVar7 != 0) {
    pcVar14 = "Wait";
    pbVar5 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar6 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar5);
    iVar7 = strcmp(pcVar6,pcVar14);
    if (iVar7 != 0) {
      pcVar14 = "Fly";
      pbVar5 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      pcVar6 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar5);
      iVar7 = strcmp(pcVar6,pcVar14);
      if (iVar7 != 0) {
        FUN_0043cc00();
        if (((((param_1[0x26] & 0x8000000U) == 0) && ((param_1[0x26] & 4U) == 0)) ||
            ((char)param_1[0x68] != '\0')) || (*(char *)((int)param_1 + 0x1a2) != '\0')) {
          local_50 = '\0';
        }
        else {
          local_50 = '\x01';
        }
        local_5 = local_50;
        if ((local_50 != '\0') &&
           (iVar7 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4), 0 < iVar7)) {
          FUN_00420da0();
          FUN_00401510(param_1 + 0x87);
          cVar4 = FUN_005b0130(0);
          if (cVar4 == '\0') {
            puVar8 = (undefined4 *)FUN_0043cb00();
            uVar10 = puVar8[1];
            uVar13 = *puVar8;
            (**(code **)(*param_1 + 4))("stand",uVar13,uVar10);
            uVar9 = FUN_004207c0();
            iVar7 = FUN_00445af0(DAT_006c55c4,param_1,uVar9,uVar13,uVar10);
            if (iVar7 == 0) {
              FUN_004405c0(&DAT_0067de1c,param_1 + 1);
            }
            else {
              FUN_004405c0("HurtSlide",param_1 + 1);
            }
            local_5 = '\0';
          }
        }
        if ((((param_1[0x26] & 0x200000U) == 0) && (local_5 == '\0')) &&
           ((char)param_1[0x29] == '\0')) {
          if (((param_1[0x26] & 0x800000U) != 0) || ((param_1[0x26] & 0x1000000U) != 0)) {
            local_18 = 0;
            local_14 = 0;
            _String_const_iterator<>();
            puVar8 = (undefined4 *)FUN_00472660(local_30);
            local_10 = *puVar8;
            local_c = puVar8[1];
            uVar10 = FUN_0046df60(local_38);
            cVar4 = FUN_00426920(uVar10);
            if (cVar4 != '\0') {
              piVar11 = (int *)FUN_004bdc80();
              if (*(int *)(*piVar11 + 8) == 0x800000) {
                local_18 = local_18 + 1;
              }
              else {
                piVar11 = (int *)FUN_004bdc80();
                if (*(int *)(*piVar11 + 8) == 0x1000000) {
                  local_14 = local_14 + 1;
                }
              }
                    /* WARNING: Subroutine does not return */
              FUN_0042f830();
            }
            fVar12 = (float10)FUN_0043c840();
            if (fVar12 <= (float10)_DAT_00697ce0) {
              bVar2 = local_18 < local_14;
            }
            else {
              bVar2 = local_14 < local_18;
            }
            if (bVar2) {
              param_1[0x19] = 0;
              _DAT_006c5598 = -_DAT_006c5598;
              pbVar5 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                       FUN_00420900();
              pcVar6 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                                 (pbVar5);
              iVar7 = strcmp("Fall",pcVar6);
              if (iVar7 != 0) {
                FUN_004405c0(&DAT_0067de3c,param_1 + 1);
              }
              fVar12 = (float10)FUN_0043c840();
              FUN_0041ffd0(fVar12 < (float10)_DAT_00697ce0);
              FUN_004026b0("GravityChange",0,0);
            }
          }
          *(bool *)((int)param_1 + 0x1a1) = (param_1[0x26] & 0x40U) != 0;
          if ((param_1[0x26] & 0x80U) == 0) {
            if ((param_1[0x26] & 0x100U) != 0) {
              (**(code **)(*DAT_006c55f0 + 0xc))(6,2);
            }
          }
          else {
            piVar11 = (int *)FUN_00461d80();
            (**(code **)(*piVar11 + 0xc))(6,1);
          }
          if ((float)_DAT_00697ce0 < (float)param_1[0x8a]) {
            fVar12 = (float10)FUN_0043e510();
            param_1[0x8a] = (int)(float)((float10)(float)param_1[0x8a] - fVar12);
          }
          iVar7 = FUN_00403bf0();
          if ((iVar7 == 0) && (((param_1[0x26] & 2U) != 0 || ((param_1[0x26] & 1U) != 0)))) {
            local_19 = '\0';
            fVar1 = (float)param_1[0x8a];
            fVar3 = (float)_DAT_00697ce0;
            if (((!NAN(fVar1) && !NAN(fVar3)) && fVar1 < fVar3 != (fVar1 == fVar3)) &&
               (iVar7 = FUN_004ac790(), iVar7 == 0xe)) {
              local_25 = '\x01';
              _String_const_iterator<>();
              puVar8 = (undefined4 *)FUN_00472660(local_40);
              local_24 = *puVar8;
              local_20 = puVar8[1];
              uVar10 = FUN_0046df60(local_48);
              cVar4 = FUN_00426920(uVar10);
              if (cVar4 != '\0') {
                if (((float)_DAT_00697ce0 < _DAT_006c5598) &&
                   ((piVar11 = (int *)FUN_004bdc80(),
                    *(float *)(*piVar11 + 4) != (float)_DAT_00697ce0 ||
                    (piVar11 = (int *)FUN_004bdc80(), (*(uint *)(*piVar11 + 8) & 1) == 0)))) {
                  local_25 = '\0';
                }
                if ((_DAT_006c5598 < (float)_DAT_00697ce0) &&
                   ((piVar11 = (int *)FUN_004bdc80(),
                    *(float *)(*piVar11 + 4) != (float)_DAT_00698f58 ||
                    (piVar11 = (int *)FUN_004bdc80(), (*(uint *)(*piVar11 + 8) & 1) == 0)))) {
                  local_25 = '\0';
                }
                    /* WARNING: Subroutine does not return */
                FUN_0042f830();
              }
              if (local_25 != '\0') {
                local_19 = '\x01';
                param_1[0x8a] = 0x3f800000;
                if (_DAT_006c5598 <= (float)_DAT_00697ce0) {
                  param_1[0x17] = (int)((float)param_1[0x17] + (float)_DAT_00697ce8);
                }
                else {
                  param_1[0x17] = (int)((float)param_1[0x17] - (float)_DAT_00697ce8);
                }
              }
            }
            if (local_19 == '\0') {
              if ((param_1[0x26] & 2U) == 0) {
                iVar7 = FUN_006253a0();
                param_1[0x16] = (int)((float)iVar7 - (float)_DAT_00697ce8);
              }
              else {
                iVar7 = FUN_006253a0();
                param_1[0x16] = (int)((float)iVar7 + (float)_DAT_00697ce8);
              }
            }
            param_1[0x18] = 0;
          }
        }
        else {
          pcVar14 = "Death";
          pbVar5 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900()
          ;
          pcVar6 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                             (pbVar5);
          iVar7 = strcmp(pcVar6,pcVar14);
          if (iVar7 != 0) {
            if ((char)param_1[0xf7] == '\0') {
              FUN_00624e20();
            }
            else {
              FUN_006249e0();
            }
            FUN_004405c0("Death",param_1 + 1);
          }
        }
      }
    }
  }
  return;
}

