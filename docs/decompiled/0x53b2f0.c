// FUN_0053b2f0 @ 0x53b2f0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0053b2f0(int *param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  bool bVar4;
  int iVar5;
  undefined4 *puVar6;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar7;
  float10 fVar8;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_148 [4];
  undefined4 uStack_144;
  char *pcStack_140;
  float fStack_13c;
  char *pcStack_138;
  char *pcVar9;
  int local_104;
  undefined1 local_f8 [16];
  undefined4 local_e8;
  undefined4 local_e4;
  undefined1 local_d8 [16];
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 *local_a0;
  undefined4 *local_9c;
  undefined1 *local_68;
  undefined1 *local_64;
  undefined4 *local_60;
  undefined4 *local_5c;
  int local_58;
  int local_54;
  int local_50;
  undefined4 local_4c;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  char local_30 [8];
  undefined4 local_28;
  undefined4 local_24;
  int local_18;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_1[0x2c] == 5) {
    return;
  }
  cVar3 = FUN_00441d90();
  if (cVar3 != '\0') {
    FUN_005adb30();
  }
  if ((param_1[4] != 0) && (cVar3 = FUN_004047f0(), cVar3 != '\0')) {
    local_60 = (undefined4 *)param_1[4];
    local_5c = local_60;
    if (local_60 != (undefined4 *)0x0) {
      (**(code **)*local_60)();
    }
    param_1[4] = 0;
    if ((char)param_1[0x2b] != '\0') {
      FUN_00539730();
    }
    *(undefined1 *)(param_1 + 0x2b) = 1;
  }
  if ((float)_DAT_00697ce0 < (float)param_1[0x29]) {
    fVar8 = (float10)FUN_0043e510();
    param_1[0x29] = (int)(float)((float10)(float)param_1[0x29] - fVar8);
    fVar1 = (float)param_1[0x29];
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      cVar3 = FUN_00403d40();
      if (cVar3 == '\0') {
        local_104 = _DAT_0069885c;
      }
      else {
        local_104 = _DAT_00698970;
      }
      param_1[0x2a] = local_104;
      cVar3 = FUN_00403d40();
      if (cVar3 == '\0') {
        pcStack_138 = "Victory";
        fStack_13c = 7.687132e-39;
        FUN_004026b0();
      }
      else {
        pcStack_138 = "VictoryZ";
        fStack_13c = 7.687102e-39;
        FUN_004026b0();
      }
      param_1[0x2c] = 2;
    }
  }
  if ((float)_DAT_00697ce0 < (float)param_1[0x2a]) {
    fVar8 = (float10)FUN_0043e510();
    param_1[0x2a] = (int)(float)((float10)(float)param_1[0x2a] - fVar8);
    fVar1 = (float)param_1[0x2a];
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      if ((param_1[2] == 0x13) && (cVar3 = FUN_00403f70(), cVar3 == '\0')) {
        *(undefined1 *)((int)param_1 + 0xad) = 1;
        FUN_005a7980();
        param_1[0x2c] = 3;
        return;
      }
      if (9 < param_1[2]) {
        FUN_005adb30();
        return;
      }
      *(undefined1 *)((int)param_1 + 0xad) = 1;
      FUN_005a7980();
      param_1[0x2c] = 3;
    }
  }
  if ((*(char *)((int)param_1 + 0xad) == '\0') || (cVar3 = FUN_00403f30(), cVar3 != '\0')) {
    if ((float)_DAT_00697ce0 < (float)param_1[7]) {
      fVar8 = (float10)FUN_0043e510();
      param_1[7] = (int)(float)((float10)(float)param_1[7] - fVar8);
      if ((float)param_1[7] < (float)_DAT_00697ce0) {
        param_1[7] = 0;
        FUN_00538fe0();
      }
    }
    local_18 = 0;
    if ((param_1[3] != 0) && (iVar5 = FUN_00403bf0(), iVar5 == 0)) {
      local_18 = (**(code **)(*param_1 + 0x14))();
    }
    if (local_18 != 0) {
      FUN_00539660();
      FUN_00562450();
      param_1[6] = param_1[5];
      FUN_0042e8a0();
      local_64 = abStack_148;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_148,"Environment");
      iVar5 = FUN_00443e30();
      param_1[5] = iVar5;
      local_68 = abStack_148;
      pbVar7 = abStack_148;
      iVar5 = param_1[1];
      (**(code **)(*param_1 + 0x18))(abStack_148,iVar5);
      FUN_005b4970(pbVar7,iVar5);
      pcStack_138 = (char *)0x53b848;
      pcStack_138 = (char *)FUN_0042e920();
      fStack_13c = 7.688475e-39;
      fVar8 = (float10)FUN_0042e900();
      fStack_13c = (float)fVar8;
      pcStack_140 = (char *)0x53b869;
      FUN_0042e8d0();
      pcStack_138 = (char *)0x53b871;
      FUN_0043bc00();
      pcStack_138 = local_30;
      fStack_13c = (float)param_1[1];
      pcStack_140 = (char *)0x53b892;
      (**(code **)(*param_1 + 0x20))();
      pcStack_140 = local_30;
      uStack_144 = 0x53b8a4;
      FUN_00403ef0();
      pcStack_140 = (char *)param_1[1];
      uStack_144 = 0x53b8c1;
      (**(code **)(*param_1 + 0x1c))();
      FUN_00445000();
      FUN_00444fa0();
      FUN_0043eef0();
      puVar6 = (undefined4 *)FUN_00472660();
      local_28 = *puVar6;
      local_24 = puVar6[1];
      while( true ) {
        FUN_0046df60();
        cVar3 = FUN_00426920();
        if (cVar3 == '\0') break;
        puVar6 = (undefined4 *)FUN_004bdc80();
        local_34 = *puVar6;
        local_38 = FUN_0042e9a0();
        FUN_004bdc80();
        iVar5 = FUN_0042e960();
        if (iVar5 == param_1[1]) {
          FUN_004bdc80();
          FUN_00621180();
        }
        else {
          iVar5 = FUN_0042e960();
          if ((iVar5 != param_1[1]) &&
             ((*(char *)(local_38 + 10) != '\0' || (*(char *)(local_38 + 0xb) != '\0')))) {
            FUN_00621290();
          }
        }
        pcStack_138 = (char *)0x53b9a8;
        FUN_0042f8a0();
      }
    }
    pcVar9 = "Teleport";
    pbVar7 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
    pcStack_138 = (undefined1 *)0x53b9c7;
    bVar4 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar7,pcVar9);
    if (bVar4) {
      FUN_00539660();
      FUN_00562450();
      FUN_0043bc00();
      puVar6 = (undefined4 *)FUN_00472660();
      local_40 = *puVar6;
      local_3c = puVar6[1];
      while( true ) {
        FUN_0046df60();
        cVar3 = FUN_00426920();
        if (cVar3 == '\0') break;
        puVar6 = (undefined4 *)FUN_004bdc80();
        local_4c = *puVar6;
        local_50 = FUN_0042e9a0();
        FUN_004bdc80();
        iVar5 = FUN_0042e960();
        if (iVar5 == param_1[1]) {
          FUN_004bdc80();
          FUN_00621180();
        }
        else {
          iVar5 = FUN_0042e960();
          if ((iVar5 != param_1[1]) &&
             ((*(char *)(local_50 + 10) != '\0' || (*(char *)(local_50 + 0xb) != '\0')))) {
            FUN_00621290();
          }
        }
        pcStack_138 = (undefined1 *)0x53bac9;
        FUN_0042f8a0();
      }
    }
    iVar5 = FUN_00403bf0();
    if ((iVar5 == 0) && (param_1[6] != 0)) {
      iVar5 = FUN_0042e940();
      if (iVar5 == 1) {
        FUN_00401930();
        FUN_005b4850();
        FUN_006253a0();
        FUN_00401930();
        FUN_006253a0();
        FUN_00445050();
      }
      else {
        FUN_00401930();
        FUN_005b4870();
        FUN_006253a0();
        FUN_00401930();
        FUN_006253a0();
        FUN_00445050();
      }
      FUN_0041efa0();
      FUN_0043e580();
      local_a0 = (undefined4 *)param_1[6];
      local_9c = local_a0;
      if (local_a0 != (undefined4 *)0x0) {
        (**(code **)*local_a0)();
      }
      param_1[6] = 0;
      FUN_00445770();
      FUN_0043e5a0();
      (**(code **)(*DAT_006c55f8 + 0x3c))();
    }
    puVar6 = (undefined4 *)FUN_00472660();
    local_c = *puVar6;
    local_8 = puVar6[1];
    while( true ) {
      FUN_0046df60();
      cVar3 = FUN_00426920();
      if (cVar3 == '\0') break;
      puVar6 = (undefined4 *)FUN_004bdc80();
      (**(code **)(*(int *)*puVar6 + 0x2c))();
      pcStack_138 = (undefined1 *)0x53bcb3;
      FUN_0042f8a0();
    }
    FUN_004b7660();
    while( true ) {
      FUN_0046a130();
      cVar3 = FUN_0046bcb0();
      if (cVar3 == '\0') break;
      FUN_004320e0();
      local_54 = FUN_0043ed20();
      if ((local_54 == 0) || (cVar3 = FUN_004047f0(), cVar3 != '\0')) {
        FUN_004320e0();
        FUN_0043e560();
        pcStack_138 = (char *)0x53bd37;
        puVar6 = (undefined4 *)FUN_0042f920();
        local_c8 = *puVar6;
        local_c4 = puVar6[1];
        pcStack_138 = local_d8;
        fStack_13c = 7.690297e-39;
        FUN_0042f050();
      }
      else {
        FUN_0042f900();
      }
    }
    FUN_004b7660();
    while( true ) {
      FUN_0046a130();
      cVar3 = FUN_0046bcb0();
      if (cVar3 == '\0') break;
      FUN_004320e0();
      local_58 = FUN_0043ed20();
      if ((local_58 == 0) || (cVar3 = FUN_004047f0(), cVar3 != '\0')) {
        FUN_004320e0();
        FUN_0043e560();
        pcStack_138 = (char *)0x53bdfc;
        puVar6 = (undefined4 *)FUN_0042f920();
        local_e8 = *puVar6;
        local_e4 = puVar6[1];
        pcStack_138 = local_f8;
        fStack_13c = 7.690573e-39;
        FUN_0042f050();
      }
      else {
        FUN_0042f900();
      }
    }
    cVar3 = FUN_00441d90();
    if (cVar3 != '\0') {
      FUN_0053b260();
    }
  }
  else {
    *(undefined1 *)((int)param_1 + 0xad) = 0;
    if (param_1[2] == 0x13) {
      FUN_0061f5c0();
      FUN_005adb30();
    }
    else {
      cVar3 = FUN_0061daf0();
      if (cVar3 == '\0') {
        cVar3 = FUN_00403d40();
        if (cVar3 == '\0') {
          FUN_0042e850();
        }
        else {
          FUN_0042e850();
        }
        FUN_0061dc50();
        if ((param_1[2] == 8) && (cVar3 = FUN_00403d40(), cVar3 != '\0')) {
          FUN_0061dc50();
        }
        else if ((param_1[2] == 3) && (cVar3 = FUN_00403d40(), cVar3 == '\0')) {
          FUN_0061dc50();
        }
      }
      FUN_005adb30();
    }
  }
  return;
}

