// FUN_005aea50 @ 0x5aefc0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005aea50(int param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  char *_Str1;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  uint uVar8;
  int iVar9;
  float10 fVar10;
  float10 fVar11;
  float10 fVar12;
  float10 fVar13;
  undefined1 *puVar14;
  undefined1 *puVar15;
  char *_Str2;
  undefined4 uVar16;
  float local_104;
  float local_100;
  float local_fc;
  float local_dc;
  float local_d8;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [24];
  undefined1 local_80 [26];
  undefined1 local_66;
  undefined1 local_65;
  float local_64;
  undefined1 local_60 [8];
  float local_58;
  float local_54;
  undefined1 local_50 [4];
  float local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [4];
  undefined1 local_34 [16];
  undefined4 local_24;
  undefined4 local_20;
  char local_19;
  undefined1 local_18 [4];
  undefined1 local_14 [8];
  int *local_c;
  
  FUN_005ad160();
  FUN_005a8730();
  _Str2 = "CometDash";
  this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00420900();
  _Str1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
  iVar4 = strcmp(_Str1,_Str2);
  if (iVar4 == 0) {
    return;
  }
  if ((*(uint *)(param_1 + 0x98) & 0x18000) != 0) {
    fVar1 = *(float *)(param_1 + 100);
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      *(undefined4 *)(param_1 + 100) = 0;
      cVar3 = FUN_00441c80(0x1e,8);
      if (cVar3 != '\0') {
        *(float *)(param_1 + 0x5c) = *(float *)(param_1 + 0x5c) + (float)_DAT_00697ce8;
      }
      if (((*(uint *)(param_1 + 0x98) & 0x8000) != 0) ||
         ((*(uint *)(param_1 + 0x98) & 0x10000) != 0)) {
        if ((*(uint *)(param_1 + 0x98) & 0x8000) == 0) {
          local_b0 = _DAT_006988d0;
        }
        else {
          local_b0 = _DAT_006987f0;
        }
        FUN_0043bc10(local_b0,0);
        uVar5 = FUN_005a7400(0,0,0);
        uVar5 = FUN_00401930(uVar5);
        FUN_005b5220(local_38,param_1,param_1 + 0x58,local_40,uVar5);
        FUN_00401510(local_34);
      }
      cVar3 = FUN_00441db0();
      if (cVar3 != '\0') {
        *(undefined1 *)(param_1 + 0x68) = 0;
        return;
      }
      cVar3 = FUN_00441db0();
      if (cVar3 == '\0') {
        return;
      }
      *(undefined1 *)(param_1 + 0x68) = 1;
      return;
    }
  }
  if (((*(char *)(param_1 + 0x3dc) != '\0') && (*(char *)(param_1 + 0x3dd) == '\0')) &&
     (*(char *)(param_1 + 0x1d8) == '\0')) {
    cVar3 = FUN_0061daf0(6);
    if ((cVar3 != '\0') && (cVar3 = FUN_005a9de0(), cVar3 != '\0')) {
      *(undefined1 *)(param_1 + 0x1d8) = 1;
      FUN_004026b0("MMWJetMissileFire",0,0);
      if (*(char *)(param_1 + 500) != '\0') {
        return;
      }
      fVar10 = (float10)FUN_0043c840();
      if ((float10)_DAT_00697ce0 <= fVar10) {
        local_b4 = 4;
      }
      else {
        local_b4 = 0xffffffec;
      }
      uVar16 = 0;
      uVar5 = FUN_0043bc20(0,0);
      uVar6 = FUN_0043bc20(5,local_b4);
      uVar6 = FUN_004014c0(local_80,uVar6);
      local_48 = FUN_0061f970(5,uVar6,uVar5,uVar16);
      FUN_00420c90(0);
      fVar10 = (float10)FUN_0043c840();
      if ((float10)_DAT_00697ce0 <= fVar10) {
        local_b8 = 4;
      }
      else {
        local_b8 = 0xffffffec;
      }
      uVar16 = 0;
      uVar5 = FUN_0043bc20(0,0);
      uVar6 = FUN_0043bc20(0xfffffffb,local_b8);
      uVar6 = FUN_004014c0(local_98,uVar6);
      local_44 = FUN_0061f970(5,uVar6,uVar5,uVar16);
      FUN_00420c90(1);
      return;
    }
    cVar3 = FUN_00441d90();
    if ((cVar3 != '\0') && (cVar3 = FUN_0061daf0(8), cVar3 != '\0')) {
      FUN_004405c0("CometDash",param_1 + 4);
      *(undefined1 *)(param_1 + 0x1d8) = 1;
      return;
    }
  }
  cVar3 = FUN_00441d90();
  if ((cVar3 != '\0') && (cVar3 = FUN_00441db0(), cVar3 == '\0')) {
    uVar5 = SDL_GetTicks();
    *(undefined4 *)(param_1 + 0x16c) = uVar5;
  }
  cVar3 = FUN_00441d90();
  if ((((cVar3 != '\0') && (cVar3 = FUN_00441db0(), cVar3 != '\0')) &&
      (cVar3 = FUN_00420f50(), cVar3 != '\0')) || (cVar3 = FUN_00441db0(), cVar3 != '\0')) {
    uVar5 = SDL_GetTicks();
    *(undefined4 *)(param_1 + 0x170) = uVar5;
  }
  if ((*(int *)(param_1 + 0x10c) == 1) && (*(char *)(param_1 + 0x19f) == '\0')) {
    *(undefined4 *)(param_1 + 100) = 0;
    if ((*(uint *)(param_1 + 0x98) & 0x1000) == 0) {
      if ((*(uint *)(param_1 + 0x98) & 0x2000) != 0) {
        *(float *)(param_1 + 0x5c) = *(float *)(param_1 + 0x5c) + (float)_DAT_00697ce8;
        *(undefined4 *)(param_1 + 0x60) = _DAT_00698390;
        *(undefined1 *)(param_1 + 0x1bc) = 1;
      }
    }
    else {
      *(float *)(param_1 + 0x5c) = *(float *)(param_1 + 0x5c) + (float)_DAT_00697ce8;
      *(undefined4 *)(param_1 + 0x60) = _DAT_00698e80;
      *(undefined1 *)(param_1 + 0x1bc) = 1;
    }
  }
  fVar10 = (float10)FUN_005a75c0(*(undefined4 *)(param_1 + 100));
  fVar11 = (float10)FUN_0043c840();
  fVar11 = (float10)FUN_005a75c0((float)fVar11);
  local_19 = (float10)(double)fVar10 == fVar11;
  if ((bool)local_19) {
    *(undefined1 *)(param_1 + 0x224) = 1;
  }
  if (*(char *)(param_1 + 0x19f) == '\0') {
    if (((*(char *)(param_1 + 0x224) != '\0') && (cVar3 = FUN_00441c80(0x1e,2), cVar3 != '\0')) &&
       (local_19 == '\0')) {
      fVar10 = (float10)FUN_005a75c0(*(undefined4 *)(param_1 + 100));
      fVar11 = (float10)FUN_0043c800();
      fVar11 = (float10)FUN_005a7590(*(undefined4 *)(param_1 + 100),(float)-fVar11);
      fVar11 = (float10)FUN_005a75f0((float)fVar11);
      *(float *)(param_1 + 100) = (float)(fVar11 * (float10)(double)fVar10);
    }
    if (*(char *)(param_1 + 0x1bc) == '\0') {
      if (((*(char *)(param_1 + 0x1cd) == '\0') && (cVar3 = FUN_00441db0(), cVar3 == '\0')) &&
         (cVar3 = FUN_00441db0(), cVar3 == '\0')) {
        fVar10 = (float10)FUN_005a7590(*(undefined4 *)(param_1 + 0x60));
        if ((float10)_DAT_00697ce0 < fVar10) {
          fVar11 = (float10)FUN_0043c800();
          fVar12 = (float10)FUN_0043e510();
          fVar10 = (float10)_DAT_006987b8;
          fVar13 = (float10)FUN_005a75c0(*(undefined4 *)(param_1 + 0x60));
          local_58 = (float)(fVar13 * (float10)(double)(fVar12 * (float10)(double)fVar11 * fVar10));
          fVar10 = (float10)FUN_005a7590(*(undefined4 *)(param_1 + 0x60));
          fVar10 = (float10)FUN_005a7590(local_58,(float)fVar10);
          fVar10 = (float10)FUN_005a75f0((float)fVar10);
          fVar11 = (float10)FUN_005a75c0(local_58);
          local_58 = (float)(fVar11 * (float10)(double)fVar10);
          *(float *)(param_1 + 0x60) = *(float *)(param_1 + 0x60) + local_58;
        }
      }
      else {
        uVar5 = 0;
        cVar3 = FUN_00441db0(0);
        FUN_0043bc20((-(uint)(cVar3 != '\0') & 0xfffffffe) + 1,uVar5);
        if (*(char *)(param_1 + 0x1cc) == '\0') {
          cVar3 = FUN_00441db0();
          *(bool *)(param_1 + 0x68) = cVar3 == '\0';
        }
        iVar4 = FUN_00401930();
        if (*(float *)(param_1 + 0x5c) < *(float *)(iVar4 + 4)) {
          iVar4 = FUN_00401930();
          local_4c = (*(float *)(iVar4 + 4) - *(float *)(param_1 + 0x5c)) + (float)_DAT_00698ad0;
        }
        uVar6 = 0;
        uVar5 = FUN_005a7400(0);
        puVar15 = local_50;
        puVar14 = local_a0;
        FUN_0043cb00(puVar14,puVar15,uVar5);
        puVar7 = (undefined4 *)FUN_004014c0(puVar14,puVar15);
        iVar4 = FUN_00445ea0(*puVar7,puVar7[1],uVar5,uVar6);
        if (iVar4 == 0) {
          fVar10 = (float10)FUN_0043c800();
          fVar11 = (float10)FUN_0043e510();
          local_54 = (float)(fVar11 * (float10)(double)fVar10);
          if (*(char *)(param_1 + 0x1cd) == '\0') {
            cVar3 = FUN_00441db0();
            if (cVar3 == '\0') {
              local_dc = 1.0;
            }
            else {
              local_dc = _DAT_00697cf0;
            }
            local_54 = local_54 * local_dc;
          }
          else {
            if (*(char *)(param_1 + 0x68) == '\0') {
              local_d8 = _DAT_00697cf0;
            }
            else {
              local_d8 = 1.0;
            }
            local_54 = local_54 * local_d8;
          }
          *(float *)(param_1 + 0x60) = *(float *)(param_1 + 0x60) + local_54;
        }
        else {
          *(undefined4 *)(param_1 + 0x60) = 0;
        }
      }
    }
  }
  else {
    uVar5 = 0;
    fVar10 = (float10)FUN_0043e510(0);
    fVar10 = (float10)FUN_005a7550((float)(fVar10 * (float10)*(float *)(param_1 + 0x60)));
    FUN_0043bc10((float)fVar10,uVar5);
    uVar6 = 0;
    uVar5 = FUN_005a7400(0);
    puVar15 = local_60;
    puVar14 = local_a8;
    FUN_0043cb00(puVar14,puVar15,uVar5);
    puVar7 = (undefined4 *)FUN_004014c0(puVar14,puVar15);
    iVar4 = FUN_00445ea0(*puVar7,puVar7[1],uVar5,uVar6);
    if (iVar4 != 0) {
      *(undefined4 *)(param_1 + 0x60) = 0;
    }
  }
  if (*(char *)(param_1 + 0x1bc) == '\0') {
    if (*(char *)(param_1 + 0x3dd) == '\0') {
      if ((*(char *)(param_1 + 0x3dc) == '\0') || (cVar3 = FUN_0061daf0(1), cVar3 == '\0')) {
        fVar10 = (float10)FUN_0043c800();
        local_104 = (float)fVar10;
      }
      else {
        local_104 = _DAT_00697fc4;
      }
      local_100 = local_104;
    }
    else {
      if ((*(char *)(param_1 + 0x3dc) == '\0') || (cVar3 = FUN_0061daf0(1), cVar3 == '\0')) {
        fVar10 = (float10)FUN_0043c800();
        local_fc = (float)fVar10;
      }
      else {
        local_fc = _DAT_00698208;
      }
      local_100 = local_fc;
    }
    local_64 = local_100;
    fVar10 = (float10)FUN_005a7490(*(undefined4 *)(param_1 + 0x60),-local_100,local_100);
    *(float *)(param_1 + 0x60) = (float)fVar10;
  }
  fVar10 = (float10)FUN_0043c840();
  fVar11 = (float10)FUN_0043e510();
  *(float *)(param_1 + 100) =
       (float)(fVar11 * (float10)(double)fVar10 + (float10)*(float *)(param_1 + 100));
  fVar10 = (float10)FUN_0043c840();
  if (fVar10 <= (float10)_DAT_00697ce0) {
    fVar10 = (float10)FUN_0043c800(*(undefined4 *)(param_1 + 100));
    fVar10 = (float10)FUN_005a7490(*(undefined4 *)(param_1 + 100),(float)-fVar10);
    *(float *)(param_1 + 100) = (float)fVar10;
  }
  else {
    fVar10 = (float10)FUN_0043c800();
    fVar10 = (float10)FUN_005a7490(*(undefined4 *)(param_1 + 100),*(undefined4 *)(param_1 + 100),
                                   (float)fVar10);
    *(float *)(param_1 + 100) = (float)fVar10;
  }
  if ((*(uint *)(param_1 + 0x98) & 0x10000000) != 0) {
    *(undefined4 *)(param_1 + 100) = _DAT_00698c18;
  }
  local_24 = *(undefined4 *)(param_1 + 0x60);
  local_20 = *(undefined4 *)(param_1 + 100);
  fVar10 = (float10)FUN_005a75c0(*(undefined4 *)(param_1 + 100));
  fVar11 = (float10)FUN_0043c840();
  fVar11 = (float10)FUN_005a75c0((float)fVar11);
  local_19 = (float10)(double)fVar10 == fVar11;
  uVar8 = FUN_005a7400(0,0,0);
  uVar5 = FUN_00401930(uVar8 | 0x20200);
  FUN_005b5220(local_18,param_1,param_1 + 0x58,&local_24,uVar5);
  FUN_00401510(local_14);
  FUN_00401510(&local_24);
  fVar10 = (float10)FUN_005a75c0(*(undefined4 *)(param_1 + 100));
  fVar11 = (float10)FUN_0043c840();
  fVar11 = (float10)FUN_005a75c0((float)fVar11);
  if ((float10)(double)fVar10 == fVar11) {
    *(undefined1 *)(param_1 + 0x224) = 1;
    *(undefined1 *)(param_1 + 0x1c4) = 1;
  }
  if (*(float *)(param_1 + 100) == (float)_DAT_00697ce0) {
    if (local_19 == '\0') {
      if ((*(uint *)(param_1 + 0x98) & 0x10000000) == 0) {
        local_65 = *(undefined1 *)(param_1 + 0x3dd);
        local_66 = *(undefined1 *)(param_1 + 0x1d8);
        FUN_004405c0(&DAT_0067dbd0,param_1 + 4);
        *(undefined1 *)(param_1 + 0x1d8) = local_66;
        *(undefined1 *)(param_1 + 0x3dd) = local_65;
      }
    }
    else {
      if (*(char *)(param_1 + 0x1a3) == '\0') {
        FUN_00420a10(&DAT_0067dbb8);
      }
      iVar9 = SDL_GetTicks();
      iVar4 = *(int *)(param_1 + 0x16c);
      fVar10 = (float10)FUN_0043c800();
      if (fVar10 <= (float10)(uint)(iVar9 - iVar4)) {
        iVar9 = SDL_GetTicks();
        iVar4 = *(int *)(param_1 + 0x170);
        fVar10 = (float10)FUN_0043c800();
        if ((fVar10 <= (float10)(uint)(iVar9 - iVar4)) || (*(char *)(param_1 + 0x3dc) != '\0')) {
          *(undefined1 *)(param_1 + 0x1cd) = 0;
          FUN_005a9770();
        }
        else {
          FUN_004405c0("Slide",param_1 + 4);
        }
      }
      else {
        fVar10 = (float10)FUN_0043c800();
        fVar11 = (float10)FUN_0043c840();
        fVar11 = (float10)FUN_005a75c0((float)fVar11);
        *(float *)(param_1 + 100) = (float)(fVar11 * (float10)(double)fVar10);
        FUN_004405c0(&DAT_0067dbc0,param_1 + 4);
      }
      if (local_c != (int *)0x0) {
        uVar5 = FUN_0041efa0();
        (**(code **)(*local_c + 0xc))(0xb,uVar5);
      }
    }
  }
  else {
    *(undefined1 *)(param_1 + 0x1a3) = 0;
    FUN_005a97d0(0);
  }
  return;
}

