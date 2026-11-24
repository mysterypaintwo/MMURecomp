// FUN_005b02f0 @ 0x5b02f0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_005b02f0(int *param_1,undefined4 param_2,float *param_3)

{
  ulonglong uVar1;
  undefined1 uVar2;
  bool bVar3;
  char cVar4;
  uint uVar5;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar6;
  char *pcVar7;
  int iVar8;
  float *pfVar9;
  undefined4 uVar10;
  undefined4 *puVar11;
  int *piVar12;
  float10 fVar13;
  undefined1 *puVar14;
  undefined4 uVar15;
  float fVar16;
  function<void___cdecl(unsigned_int_const&)> *pfVar17;
  char *pcVar18;
  undefined4 local_16c;
  int local_164;
  undefined4 local_160;
  undefined1 local_134 [8];
  undefined1 local_12c [8];
  undefined1 local_124 [8];
  undefined1 local_11c [8];
  undefined1 local_114 [8];
  undefined1 local_10c [8];
  void *local_104;
  undefined1 local_100 [8];
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
  undefined1 local_e8 [8];
  undefined1 local_e0 [8];
  undefined1 local_d8 [8];
  undefined1 local_d0 [16];
  undefined1 local_c0 [16];
  undefined4 local_b0;
  float local_a4;
  float local_a0;
  int local_9c;
  function<void___cdecl(unsigned_int_const&)> local_98 [28];
  float *local_7c;
  float local_78;
  float local_74;
  char local_6d;
  undefined4 local_6c;
  undefined4 local_68;
  function<void___cdecl(unsigned_int_const&)> local_64 [28];
  undefined4 local_48;
  undefined4 local_44;
  float *local_40;
  char local_39;
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  int local_2c;
  float local_28;
  int *local_24;
  undefined4 local_20;
  float local_1c;
  float local_18;
  bool local_12;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00640abe;
  local_10 = ExceptionList;
  uVar5 = DAT_006c1e94 ^ (uint)(local_64 + 0x60);
  ExceptionList = &local_10;
  local_11 = '\0';
  pcVar18 = "Spawn";
  pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
           FUN_00420900("Spawn",uVar5);
  pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
  iVar8 = strcmp(pcVar7,pcVar18);
  if (iVar8 == 0) {
    ExceptionList = local_10;
    return;
  }
  pcVar18 = "Fly";
  pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
           FUN_00420900(&DAT_0067de5c,uVar5);
  pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
  iVar8 = strcmp(pcVar7,pcVar18);
  if (iVar8 == 0) {
    ExceptionList = local_10;
    return;
  }
  switch(param_2) {
  case 0:
    goto switchD_005b039d_caseD_0;
  case 1:
    if ((char)param_1[0xf7] == '\0') {
      FUN_00624e20();
    }
    else {
      FUN_006249e0();
    }
    FUN_004405c0("Death",param_1 + 1);
    break;
  case 5:
    param_1[0x6b] = (int)param_3;
    switch(param_1[0x6b]) {
    case 1:
    case 2:
      param_1[0x6c] = _DAT_006988d0;
      break;
    case 3:
    case 4:
      param_1[0x6c] = _DAT_00698c3c;
      break;
    case 5:
    case 6:
      param_1[0x6c] = _DAT_00698f50;
      param_1[0x6b] = (param_1[0x6b] != 5) + 3;
    }
    param_1[0x6d] = param_1[0x6c];
    FUN_004405c0("Transition",param_1 + 1);
    break;
  case 7:
    local_40 = param_3;
    FUN_00427c40();
    local_8 = 0;
    _String_const_iterator<>();
    local_39 = '\x01';
    local_48 = *(undefined4 *)*local_40;
    local_44 = ((undefined4 *)*local_40)[1];
    pfVar17 = local_64;
    uVar15 = 1;
    fVar16 = *local_40;
    puVar14 = local_e0;
    uVar10 = FUN_00401930(puVar14,fVar16,1,pfVar17);
    FUN_00403d90(local_d8,uVar10);
    puVar11 = (undefined4 *)FUN_004014c0(puVar14,fVar16);
    FUN_00445ea0(*puVar11,puVar11[1],uVar15,pfVar17);
    puVar11 = (undefined4 *)FUN_00472660(local_e8);
    local_38 = *puVar11;
    local_34 = puVar11[1];
    uVar10 = FUN_0046df60(local_f0);
    cVar4 = FUN_00426920(uVar10);
    if (cVar4 != '\0') {
      piVar12 = (int *)FUN_004bdc80();
      if (*(float *)(*piVar12 + 0xc) != local_40[1]) {
        local_39 = '\0';
      }
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
    if (local_39 != '\0') {
      param_1[0x16] = (int)((float)param_1[0x16] + *(float *)*local_40);
      param_1[0x17] = (int)((float)param_1[0x17] + *(float *)((int)*local_40 + 4));
    }
    puVar11 = (undefined4 *)FUN_00472660(local_f8);
    local_38 = *puVar11;
    local_34 = puVar11[1];
    uVar10 = FUN_0046df60(local_100);
    cVar4 = FUN_00426920(uVar10);
    if (cVar4 != '\0') {
      puVar11 = (undefined4 *)FUN_004bdc80();
      local_104 = (void *)*puVar11;
      operator_delete(local_104);
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
    local_8 = 0xffffffff;
    std::function<void___cdecl(unsigned_int_const&)>::~function<void___cdecl(unsigned_int_const&)>
              (local_64);
    break;
  case 8:
    if ((char)param_1[0x71] == '\0') {
      ExceptionList = local_10;
      return;
    }
    pcVar18 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
    iVar8 = strcmp(pcVar7,pcVar18);
    if (iVar8 == 0) {
      ExceptionList = local_10;
      return;
    }
    local_7c = param_3;
    local_a4 = *(float *)*param_3;
    local_a0 = ((float *)*param_3)[1];
    local_9c = FUN_0043ed20();
    if ((param_1[0x70] != 0) && (param_1[0x70] != local_9c)) {
      uVar10 = FUN_0041efa0();
      (**(code **)(*(int *)param_1[0x70] + 0xc))(0xc,uVar10);
    }
    if ((float)param_1[0x16] != local_a4) {
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
      iVar8 = strcmp("Hurt",pcVar7);
      if (iVar8 != 0) {
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
        pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6)
        ;
        iVar8 = strcmp("Ride",pcVar7);
        if (iVar8 != 0) {
          pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900()
          ;
          pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                             (pbVar6);
          iVar8 = strcmp("ZeroMessenko",pcVar7);
          if (iVar8 != 0) {
            FUN_004405c0(&DAT_0067df5c,param_1 + 1);
          }
        }
      }
    }
    *(undefined1 *)((int)param_1 + 0x1bd) = 1;
    param_1[0x70] = local_9c;
    uVar10 = FUN_0041efa0();
    (**(code **)(*(int *)param_1[0x70] + 0xc))(0xb,uVar10);
    fVar13 = (float10)FUN_0043c840();
    if (fVar13 <= (float10)_DAT_00697ce0) {
      local_16c = _DAT_00698d0c;
    }
    else {
      local_16c = 0;
    }
    FUN_0043bc10(0,local_16c);
    FUN_00427c40();
    local_8 = 1;
    _String_const_iterator<>();
    local_6d = '\x01';
    pfVar17 = local_98;
    uVar10 = FUN_005a7400(pfVar17);
    pfVar9 = &local_78;
    puVar14 = local_114;
    uVar15 = FUN_00401930(puVar14,pfVar9,uVar10);
    FUN_00403d90(local_10c,uVar15);
    puVar11 = (undefined4 *)FUN_004014c0(puVar14,pfVar9);
    FUN_00445ea0(*puVar11,puVar11[1],uVar10,pfVar17);
    puVar11 = (undefined4 *)FUN_00472660(local_11c);
    local_6c = *puVar11;
    local_68 = puVar11[1];
    uVar10 = FUN_0046df60(local_124);
    cVar4 = FUN_00426920(uVar10);
    if (cVar4 != '\0') {
      piVar12 = (int *)FUN_004bdc80();
      if (*(float *)(*piVar12 + 0xc) != local_7c[1]) {
        local_6d = '\0';
      }
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
    puVar11 = (undefined4 *)FUN_00472660(local_12c);
    local_6c = *puVar11;
    local_68 = puVar11[1];
    uVar10 = FUN_0046df60(local_134);
    cVar4 = FUN_00426920(uVar10);
    if (cVar4 != '\0') {
      puVar11 = (undefined4 *)FUN_004bdc80();
      operator_delete((void *)*puVar11);
                    /* WARNING: Subroutine does not return */
      FUN_0042f830();
    }
    if (local_6d != '\0') {
      param_1[0x16] = (int)(local_a4 - local_78);
    }
    param_1[0x17] = (int)(local_a0 - local_74);
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
    iVar8 = strcmp("Fall",pcVar7);
    if (iVar8 == 0) {
LAB_005b1199:
      FUN_00420a10(&DAT_0067df7c);
      cVar4 = FUN_00441db0();
      if ((cVar4 == '\0') && (cVar4 = FUN_00441db0(), cVar4 == '\0')) {
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
        pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6)
        ;
        iVar8 = strcmp("Stand",pcVar7);
        if (iVar8 != 0) {
          FUN_004405c0("Stand",param_1 + 1);
        }
      }
      else {
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
        pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6)
        ;
        iVar8 = strcmp("Run",pcVar7);
        if (iVar8 != 0) {
          FUN_004405c0(&DAT_0067df88,param_1 + 1);
        }
      }
    }
    else {
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
      iVar8 = strcmp("ZeroShoryuken",pcVar7);
      if (iVar8 == 0) goto LAB_005b1199;
    }
    local_8 = 0xffffffff;
    std::function<void___cdecl(unsigned_int_const&)>::~function<void___cdecl(unsigned_int_const&)>
              (local_98);
    break;
  case 9:
    pcVar18 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
    iVar8 = strcmp(pcVar7,pcVar18);
    if (iVar8 != 0) {
      *(undefined1 *)((int)param_1 + 0x1bd) = 0;
      param_1[0x70] = 0;
      pcVar7 = "Transition";
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      bVar3 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
      if (bVar3) {
        pcVar7 = "Ladder";
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
        bVar3 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
        if (bVar3) {
          pcVar7 = "LadderOut";
          pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900()
          ;
          bVar3 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
          if (bVar3) {
            pcVar7 = "HurtKnockback";
            pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                     FUN_00420900();
            bVar3 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>
                              (pbVar6,pcVar7);
            if (bVar3) {
              pcVar7 = "Glued";
              pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                       FUN_00420900();
              bVar3 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>
                                (pbVar6,pcVar7);
              if (bVar3) {
                pcVar7 = "Teleport";
                pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                         FUN_00420900();
                bVar3 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>
                                  (pbVar6,pcVar7);
                if (bVar3) {
                  pcVar7 = "ZeroShoryuken";
                  pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                           FUN_00420900();
                  bVar3 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>
                                    (pbVar6,pcVar7);
                  if (bVar3) {
                    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                             FUN_00420900();
                    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                             c_str(pbVar6);
                    iVar8 = strcmp("Fall",pcVar7);
                    if (iVar8 != 0) {
                      FUN_004405c0(&DAT_0067e000,param_1 + 1);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    break;
  case 0xd:
    FUN_004026b0("EnergyMeter3s",0,0);
    param_1[99] = param_1[99] + (int)param_3;
    FUN_005a8870();
    break;
  case 0xe:
    FUN_004026b0("EnergyMeter3s",0,0);
    param_1[100] = param_1[100] + (int)param_3;
    if ((char)param_1[0xf7] == '\0') {
      local_164 = FUN_00425990();
    }
    else {
      local_164 = 10;
    }
    param_1[0x65] = local_164;
    fVar13 = (float10)FUN_00420e80(param_1[0x65]);
    if (((float10)_DAT_00698b48 <= fVar13) &&
       (iVar8 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4), 0 < iVar8)) {
      local_28 = _DAT_00698a88;
      for (local_2c = 0; local_2c < 0xd; local_2c = local_2c + 1) {
        fVar13 = (float10)FUN_00420e80(local_2c);
        local_30 = (float)fVar13;
        if (local_30 < local_28) {
          param_1[0x65] = local_2c;
          local_28 = local_30;
        }
      }
    }
    FUN_005a8870();
    break;
  case 0xf:
    uVar2 = FUN_00420b00();
    FUN_00420b60(uVar2);
    FUN_004405c0(&DAT_0067e008,param_1 + 1);
    break;
  case 0x11:
    pcVar18 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
    iVar8 = strcmp(pcVar7,pcVar18);
    if (iVar8 != 0) {
      pcVar18 = "Glued";
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
      iVar8 = strcmp(pcVar7,pcVar18);
      if (iVar8 != 0) {
        param_1[0x72] = (int)*param_3;
        FUN_004405c0("Glued",param_1 + 1);
      }
    }
    break;
  case 0x13:
    pcVar18 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
    iVar8 = strcmp(pcVar7,pcVar18);
    if (iVar8 != 0) {
      param_1[0x72] = 0;
    }
    break;
  case 0x14:
    pcVar18 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
    iVar8 = strcmp(pcVar7,pcVar18);
    if (iVar8 != 0) {
      pcVar18 = "Fall";
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
      iVar8 = strcmp(pcVar7,pcVar18);
      if (iVar8 != 0) {
        pcVar18 = "ZeroShoryuken";
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
        pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6)
        ;
        iVar8 = strcmp(pcVar7,pcVar18);
        if (iVar8 != 0) {
          pcVar18 = "Hurt";
          pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900()
          ;
          pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                             (pbVar6);
          iVar8 = strcmp(pcVar7,pcVar18);
          if (iVar8 != 0) {
            pcVar18 = "Stun";
            pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
                     FUN_00420900();
            pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str
                               (pbVar6);
            iVar8 = strcmp(pcVar7,pcVar18);
            if (iVar8 != 0) {
              param_1[0x75] = (int)*param_3;
              FUN_004405c0(&DAT_0067def8,param_1 + 1);
            }
          }
        }
      }
    }
    break;
  case 0x1a:
    fVar13 = (float10)FUN_0043c840();
    fVar13 = (float10)FUN_005a75c0((float)fVar13);
    param_1[0x19] = (int)(float)(fVar13 * (float10)*param_3);
    *(undefined1 *)(param_1 + 0x89) = 0;
    FUN_004405c0(&DAT_0067e00c,param_1 + 1);
    break;
  case 0x21:
    local_11 = '\x01';
    *(undefined1 *)(param_1 + 0x76) = 0;
    *(undefined1 *)((int)param_1 + 0x3dd) = 0;
switchD_005b039d_caseD_0:
    iVar8 = FUN_00436980();
    if (iVar8 != 0) {
      FUN_00436980();
      cVar4 = FUN_004047f0();
      if (cVar4 != '\0') {
        ExceptionList = local_10;
        return;
      }
    }
    if ((local_11 == '\0') && ((char)param_1[0x7b] != '\0')) {
      ExceptionList = local_10;
      return;
    }
    if ((char)param_1[0x68] != '\0') {
      ExceptionList = local_10;
      return;
    }
    if (param_1[0x54] == 0) {
      ExceptionList = local_10;
      return;
    }
    if (*(char *)((int)param_1 + 0x1a2) != '\0') {
      ExceptionList = local_10;
      return;
    }
    local_18 = *param_3;
    uVar1 = (ulonglong)(uint)local_18;
    fVar13 = (float10)FUN_00420f10();
    FUN_005a7620(0x3f800000,(float)(fVar13 * (float10)uVar1));
    local_20 = FUN_006253a0();
    if (local_18 == 0.0) {
      local_20 = 0;
    }
    pfVar9 = (float *)FUN_00420a40();
    local_1c = *pfVar9;
    local_12 = SUB41(param_1[0x1a],0);
    if (local_1c == (float)_DAT_00697ce0) {
      pfVar9 = (float *)FUN_00401930();
      bVar3 = *pfVar9 <= (float)param_1[0x16];
    }
    else {
      bVar3 = (float)_DAT_00697ce0 <= local_1c;
    }
    local_12 = !bVar3;
    if (((char)param_1[0xf7] != '\0') && (cVar4 = FUN_0061daf0(3), cVar4 != '\0')) {
      pcVar7 = "Stand";
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
      bVar3 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
      if (!bVar3) {
        pcVar7 = "Ride";
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
        bVar3 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
        if (!bVar3) goto LAB_005b07b7;
      }
      if (((local_12 == (bool)(char)param_1[0x1a]) && (cVar4 = FUN_0043c860(), cVar4 != '\0')) &&
         (fVar13 = (float10)FUN_0043c800(), fVar13 == (float10)_DAT_00697ce8)) {
        FUN_004026b0("ShotDeflect",0,0);
        FUN_004047a0(1);
        uVar10 = (**(code **)(*param_1 + 4))("Deflect");
        (**(code **)(*param_1 + 0x10))(uVar10);
        if ((char)param_1[0x1a] == '\0') {
          uVar10 = FUN_0043bc20(10,0xf);
          local_160 = FUN_00403d90(local_d0,uVar10);
        }
        else {
          uVar10 = FUN_0043bc20(10,0xfffffff1);
          local_160 = FUN_004014c0(local_c0,uVar10);
        }
        local_b0 = local_160;
        uVar15 = 0;
        uVar10 = FUN_0043bc20(0,0);
        FUN_0061f970(1,local_b0,uVar10,uVar15);
        if (param_1[0x5d] != -1) {
          FUN_004467d0(param_1[0x5d]);
          param_1[0x5d] = -1;
        }
        *(undefined1 *)((int)param_1 + 0x19e) = 1;
        *(undefined1 *)((int)param_1 + 0x19d) = 0;
        iVar8 = FUN_00446440(_DAT_00698f54,1,param_1,FUN_005a9490);
        param_1[0x5d] = iVar8;
        ExceptionList = local_10;
        return;
      }
    }
LAB_005b07b7:
    *(bool *)(param_1 + 0x1a) = local_12;
    cVar4 = FUN_005b0130(local_20);
    if (cVar4 == '\0') {
      if (local_11 == '\0') {
        cVar4 = FUN_005aa3c0();
        if (cVar4 == '\0') {
          FUN_004405c0("HurtSlide",param_1 + 1);
        }
        else {
          FUN_004405c0(&DAT_0067dea8,param_1 + 1);
        }
      }
      else {
        local_24 = (int *)param_3[1];
        (**(code **)(*local_24 + 0xc))(0x27,0);
        FUN_004405c0("HurtKnockback",param_1 + 1);
      }
    }
    break;
  case 0x22:
    FUN_00403220(1);
    break;
  case 0x23:
    pcVar7 = "Carried";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar3 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if ((bVar3) && (cVar4 = FUN_005a9c40(1), cVar4 == '\0')) {
      *(undefined1 *)(param_1 + 0x7b) = 0;
      FUN_004405c0("Stand",param_1 + 1);
    }
    break;
  case 0x24:
    FUN_004405c0("Teleport",param_1 + 1);
    break;
  case 0x26:
    FUN_00420ac0(0);
  }
  ExceptionList = local_10;
  return;
}

