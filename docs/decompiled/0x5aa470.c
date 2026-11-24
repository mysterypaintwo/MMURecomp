// FUN_005aa470 @ 0x5aa470

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005aa470(int param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  bool bVar4;
  char cVar5;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *pbVar6;
  char *pcVar7;
  int iVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  float *pfVar11;
  undefined4 *puVar12;
  float10 fVar13;
  undefined1 *puVar14;
  undefined1 *puVar15;
  char *pcVar16;
  undefined4 uVar17;
  undefined4 local_98;
  char local_94;
  undefined4 local_90;
  undefined1 local_88 [8];
  undefined1 local_80 [24];
  undefined1 local_68 [24];
  undefined1 local_50 [27];
  char local_35;
  undefined1 local_34 [8];
  undefined4 local_2c;
  undefined4 local_28;
  int *local_24;
  undefined4 local_20;
  undefined4 local_1c;
  float local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (*(char *)(param_1 + 0x19d) == '\0') {
    FUN_005a9390(*(undefined4 *)(param_1 + 0x174));
  }
  FUN_00401510(param_1 + 0x58);
  if ((((*(char *)(param_1 + 0x230) == '\0') && (cVar3 = FUN_00441db0(), cVar3 != '\0')) &&
      (cVar3 = FUN_00441db0(), cVar3 != '\0')) && (cVar3 = FUN_00441db0(), cVar3 != '\0')) {
    pcVar16 = "Wait";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
    iVar8 = strcmp(pcVar7,pcVar16);
    if (iVar8 != 0) {
      *(undefined1 *)(param_1 + 0x230) = 1;
    }
  }
  pcVar16 = "Wait";
  pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
  pcVar7 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(pbVar6);
  iVar8 = strcmp(pcVar7,pcVar16);
  if (iVar8 == 0) {
    return;
  }
  if (*(char *)(param_1 + 500) != '\0') {
    pcVar7 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if (bVar4) {
      pcVar7 = "Transition";
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
      if (bVar4) {
        fVar13 = (float10)FUN_0043e510();
        *(float *)(param_1 + 0x1f8) = (float)((float10)*(float *)(param_1 + 0x1f8) - fVar13);
        fVar1 = *(float *)(param_1 + 0x1f8);
        fVar2 = (float)_DAT_00697ce0;
        if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
          *(undefined4 *)(param_1 + 0x1f8) = _DAT_00698938;
          if (*(float *)(param_1 + 0x22c) == (float)_DAT_00697ce0) {
            uVar17 = 0;
            uVar9 = FUN_0043bc10(0,_DAT_006987f0);
            uVar10 = FUN_0043bc10(0,_DAT_006989b0);
            uVar10 = FUN_004014c0(local_50,uVar10);
            local_8 = FUN_0061f970(9,uVar10,uVar9,uVar17);
          }
          else {
            if (*(float *)(param_1 + 0x22c) <= (float)_DAT_00697ce0) {
              local_90 = _DAT_006987f0;
            }
            else {
              local_90 = _DAT_006988d0;
            }
            uVar17 = 0;
            uVar9 = FUN_0043bc10(local_90,_DAT_006987f0);
            uVar10 = FUN_0043bc10(0,_DAT_006989b0);
            uVar10 = FUN_004014c0(local_68,uVar10);
            local_c = FUN_0061f970(9,uVar10,uVar9,uVar17);
          }
        }
      }
    }
  }
  cVar3 = *(char *)(param_1 + 500);
  cVar5 = FUN_0043c890();
  if (cVar3 != cVar5) {
    pcVar7 = "Fall";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar4 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if (!bVar4) {
      pcVar7 = "Hurt";
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      bVar4 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
      if (!bVar4) {
        pcVar7 = "ZeroShoryuken";
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
        bVar4 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
        if (!bVar4) {
          pcVar7 = "ZeroMessenkoDrop";
          pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900()
          ;
          bVar4 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
          if (!bVar4) goto LAB_005aa878;
        }
      }
    }
    local_10 = 10;
    local_14 = FUN_0043d3a0(0x2000000,3,10);
    if (local_14 < 0xe0) {
      local_1c = *(undefined4 *)(param_1 + 0x58);
      local_18 = (float)local_14 + *(float *)(param_1 + 0x5c);
      uVar10 = 0;
      uVar9 = FUN_0043bc20(0,0);
      local_20 = FUN_0061f970(6,&local_1c,uVar9,uVar10);
      FUN_004026b0("WaterSplash",0,0);
      *(char *)(param_1 + 500) = '\x01' - (*(char *)(param_1 + 500) != '\0');
    }
  }
LAB_005aa878:
  FUN_0043cf60();
  FUN_0061b670();
  FUN_005625d0();
  if ((((*(char *)(param_1 + 0x1a1) == '\0') &&
       (fVar13 = (float10)FUN_0043c840(), (float10)_DAT_00697ce0 < fVar13)) &&
      (iVar8 = FUN_00401930(),
      (float)_DAT_00698e48 < *(float *)(param_1 + 0x5c) - *(float *)(iVar8 + 4))) ||
     ((fVar13 = (float10)FUN_0043c840(), fVar13 < (float10)_DAT_00697ce0 &&
      (iVar8 = FUN_00401930(),
      *(float *)(param_1 + 0x5c) - *(float *)(iVar8 + 4) < (float)_DAT_00698e40)))) {
    iVar8 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
    if ((0 < iVar8) &&
       (((fVar13 = (float10)FUN_0043c840(), (float10)_DAT_00697ce0 < fVar13 &&
         (pfVar11 = (float *)FUN_00401930(),
         (float)_DAT_00697ce0 < *(float *)(param_1 + 0x58) - *pfVar11)) &&
        (pfVar11 = (float *)FUN_00401930(),
        *(float *)(param_1 + 0x58) - *pfVar11 < (float)_DAT_006987d0)))) {
      FUN_00441b70(1);
      local_24 = (int *)FUN_0043ed20();
      if (local_24 != (int *)0x0) {
        (**(code **)(*local_24 + 0xc))(0xf,0);
        pcVar7 = "Carried";
        pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
        bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
        if (bVar4) {
          FUN_00420d30();
          FUN_004405c0("Carried",param_1 + 4);
          FUN_00403220(0);
          return;
        }
        return;
      }
      iVar8 = FUN_00401930();
      FUN_0043bc10(*(undefined4 *)(param_1 + 0x58),*(undefined4 *)(iVar8 + 4));
      uVar9 = 0;
      puVar12 = (undefined4 *)FUN_0043bc20(0,0);
      local_24 = (int *)FUN_005629b0(local_2c,local_28,0x34,*puVar12,puVar12[1],uVar9);
      if (local_24 != (int *)0x0) {
        uVar9 = FUN_0041efa0();
        *(undefined4 *)(param_1 + 0x218) = uVar9;
        FUN_00420d30();
        FUN_004405c0("Carried",param_1 + 4);
        FUN_00403220(0);
        return;
      }
      local_24 = (int *)0x0;
    }
    pcVar7 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if (!bVar4) {
      return;
    }
    pcVar7 = "Carried";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if (bVar4) {
      FUN_004405c0("Death",param_1 + 4);
      return;
    }
    return;
  }
  if ((*(char *)(param_1 + 0x1a0) != '\0') && (*(char *)(param_1 + 0x19f) == '\0')) {
    pcVar7 = "Death";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
    bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if (bVar4) {
      fVar13 = (float10)FUN_0043e510();
      *(float *)(param_1 + 0x198) = (float)(fVar13 + (float10)*(float *)(param_1 + 0x198));
      if ((float)_DAT_006989a0 < *(float *)(param_1 + 0x198)) {
        cVar3 = FUN_0043a010();
        FUN_00403220(cVar3 == '\0');
        *(float *)(param_1 + 0x198) = *(float *)(param_1 + 0x198) - (float)_DAT_006989a0;
      }
    }
  }
  cVar3 = FUN_00441d90();
  if (cVar3 != '\0') {
    FUN_004405c0(&DAT_0067ddac,param_1 + 4);
  }
  cVar3 = FUN_00441d90();
  if (cVar3 != '\0') {
    *(char *)(param_1 + 0x1a2) = '\x01' - (*(char *)(param_1 + 0x1a2) != '\0');
  }
  if (*(char *)(param_1 + 0x1cc) == '\0') {
    return;
  }
  if (*(int *)(param_1 + 0x1ac) != 0) {
    return;
  }
  if (*(float *)(param_1 + 0x58) < *(float *)(param_1 + 0x1d0) - (float)_DAT_00698458) {
LAB_005aacf1:
    local_94 = '\0';
  }
  else {
    fVar2 = *(float *)(param_1 + 0x1d0) + (float)_DAT_00698458;
    fVar1 = *(float *)(param_1 + 0x58);
    if ((NAN(fVar1) || NAN(fVar2)) || fVar1 < fVar2 == (fVar1 == fVar2)) goto LAB_005aacf1;
    local_94 = '\x01';
  }
  local_35 = local_94;
  if ((local_94 != '\0') ||
     ((*(char *)(param_1 + 0x68) != '\0' &&
      (*(float *)(param_1 + 0x1d0) - (float)_DAT_00698458 <= *(float *)(param_1 + 0x58))))) {
LAB_005aad78:
    *(undefined4 *)(param_1 + 0x58) = *(undefined4 *)(param_1 + 0x1d0);
    if (*(char *)(param_1 + 0x1c5) != '\0') {
      *(undefined1 *)(param_1 + 0x1cc) = 0;
      return;
    }
    pcVar7 = "Fall";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if (bVar4) {
      pcVar7 = "WeaponAbsorb";
      pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
      bVar4 = std::operator!=<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
      if (bVar4) {
        *(undefined1 *)(param_1 + 0x68) = 1;
        if (*(char *)(param_1 + 0x1ff) != '\0') {
          FUN_004405c0("WeaponAbsorb",param_1 + 4);
          return;
        }
        FUN_00441b70(0);
        *(undefined1 *)(param_1 + 0x1cc) = 0;
        FUN_004405c0("Stand",param_1 + 4);
      }
    }
  }
  else {
    if (*(char *)(param_1 + 0x68) == '\0') {
      fVar2 = *(float *)(param_1 + 0x1d0) + (float)_DAT_00698458;
      fVar1 = *(float *)(param_1 + 0x58);
      if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) goto LAB_005aad78;
    }
    pcVar7 = "Stand";
    pbVar6 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
    bVar4 = std::operator==<char,std::char_traits<char>,std::allocator<char>_>(pbVar6,pcVar7);
    if (bVar4) {
      FUN_004405c0(&DAT_0067ddec,param_1 + 4);
    }
  }
  if (*(char *)(param_1 + 0x68) == '\0') {
    local_98 = _DAT_006989c8;
  }
  else {
    local_98 = _DAT_00698c28;
  }
  FUN_0043bc10(local_98,0);
  uVar17 = 0;
  uVar10 = 1;
  puVar15 = local_34;
  puVar14 = local_88;
  uVar9 = FUN_00401930(puVar14,puVar15,1,0);
  FUN_00403d90(local_80,uVar9);
  puVar12 = (undefined4 *)FUN_004014c0(puVar14,puVar15);
  iVar8 = FUN_00445ea0(*puVar12,puVar12[1],uVar10,uVar17);
  if (iVar8 != 0) {
    *(undefined1 *)(param_1 + 0x1cd) = 1;
    FUN_005a9de0();
  }
  return;
}

