// FUN_0050f7d0 @ 0x50f7d0

void __thiscall FUN_0050f7d0(undefined4 *param_1,int param_2)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  void *pvVar5;
  undefined4 local_1d8;
  undefined4 local_1d4;
  undefined4 local_1d0;
  undefined4 local_1cc;
  undefined4 local_1c8;
  undefined4 local_1c4;
  undefined4 local_1c0;
  undefined4 local_1bc;
  undefined4 local_1b8;
  undefined4 local_1b4;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  undefined4 local_19c;
  undefined4 local_198;
  undefined4 local_194;
  undefined4 local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_148 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_12c [28];
  void *local_110;
  void *local_108;
  void *local_100;
  void *local_f8;
  void *local_f0;
  void *local_e8;
  void *local_e0;
  void *local_d8;
  void *local_d0;
  void *local_c8;
  void *local_c0;
  undefined4 local_bc;
  void *local_b8;
  undefined4 local_b4;
  void *local_b0;
  void *local_a8;
  void *local_a0;
  void *local_98;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_90 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_74 [28];
  void *local_58;
  void *local_50;
  undefined4 local_4c;
  void *local_48;
  undefined4 local_44;
  void *local_40;
  undefined4 local_3c;
  void *local_38;
  undefined4 local_34;
  void *local_30;
  undefined4 local_2c;
  void *local_28;
  void *local_20;
  undefined4 *local_18;
  undefined4 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00641387;
  local_10 = ExceptionList;
  uVar2 = DAT_006c1e94 ^ (uint)(local_74 + 0x70);
  ExceptionList = &local_10;
  if (param_2 == 0xe) {
    FUN_00507f00(0);
    FUN_00441b70(0);
  }
  else if (param_2 == 0xf) {
    FUN_00507f00(0);
    FUN_00441b70(0);
    FUN_00462a90();
    FUN_005adb30();
  }
  else {
    if ((*(char *)(param_1 + 0x17) != '\0') && (param_1[7] != 0)) {
      FUN_00507f00(0);
      FUN_00507fd0(1);
      *param_1 = 4;
    }
    if (param_1[6] != 0) {
      local_18 = (undefined4 *)param_1[6];
      local_14 = local_18;
      if (local_18 != (undefined4 *)0x0) {
        (**(code **)*local_18)(1,uVar2);
      }
      param_1[6] = 0;
    }
    switch(param_2) {
    case 1:
      local_20 = operator_new(0x64c);
      local_8 = 0;
      if (local_20 == (void *)0x0) {
        local_170 = 0;
      }
      else {
        local_170 = FUN_004f33a0();
      }
      param_1[6] = local_170;
      break;
    case 2:
      local_30 = operator_new(0x17d4);
      local_8 = 2;
      if (local_30 == (void *)0x0) {
        local_178 = 0;
      }
      else {
        uVar4 = FUN_00508ea0();
        local_178 = FUN_00503890(uVar4);
      }
      local_2c = local_178;
      local_8 = 0xffffffff;
      param_1[6] = local_178;
      cVar1 = FUN_00443240();
      if (cVar1 == '\0') {
        FUN_004026b0("MenuThemeB",1,1);
      }
      break;
    case 3:
      cVar1 = FUN_00403d40();
      if (cVar1 == '\0') {
        local_38 = operator_new(0x17d4);
        local_8 = 3;
        if (local_38 == (void *)0x0) {
          local_17c = 0;
        }
        else {
          uVar4 = FUN_00508d00();
          local_17c = FUN_00503890(uVar4);
        }
        local_34 = local_17c;
        param_1[6] = local_17c;
      }
      else {
        local_40 = operator_new(0x17d4);
        local_8 = 4;
        if (local_40 == (void *)0x0) {
          local_180 = 0;
        }
        else {
          uVar4 = FUN_00508820();
          local_180 = FUN_00503890(uVar4);
        }
        local_3c = local_180;
        param_1[6] = local_180;
      }
      local_8 = 0xffffffff;
      cVar1 = FUN_00443240();
      if (cVar1 == '\0') {
        FUN_004026b0("MenuThemeB",1,1);
      }
      break;
    case 4:
      cVar1 = FUN_00403d40();
      if (cVar1 == '\0') {
        local_48 = operator_new(0x17d4);
        local_8 = 5;
        if (local_48 == (void *)0x0) {
          local_184 = 0;
        }
        else {
          uVar4 = FUN_00508b60();
          local_184 = FUN_00503890(uVar4);
        }
        local_44 = local_184;
        param_1[6] = local_184;
      }
      else {
        local_50 = operator_new(0x17d4);
        local_8 = 6;
        if (local_50 == (void *)0x0) {
          local_188 = 0;
        }
        else {
          uVar4 = FUN_00508680();
          local_188 = FUN_00503890(uVar4);
        }
        local_4c = local_188;
        param_1[6] = local_188;
      }
      local_8 = 0xffffffff;
      FUN_00430a90(1);
      break;
    case 5:
      local_58 = operator_new(0xa50);
      local_8 = 7;
      if (local_58 == (void *)0x0) {
        local_18c = 0;
      }
      else {
        local_18c = FUN_004feb80(param_1[10]);
      }
      param_1[6] = local_18c;
      break;
    case 6:
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::clear
                ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x10)
                );
      *(undefined1 *)(param_1 + 0x18) = 1;
      iVar3 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
      if (((iVar3 < 2) && (param_1[0xf] != 9)) && (cVar1 = FUN_00403d40(), cVar1 == '\0')) {
        iVar3 = FUN_0061dca0();
        if ((iVar3 == 4) && (cVar1 = FUN_004309f0(), cVar1 == '\0')) {
          FUN_00430a10(1);
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          basic_string<char,std::char_traits<char>,std::allocator<char>_>
                    (local_74,"..\\Cutscenes\\CSN02FourBosses\\CSN02FourBosses.xcut");
          local_8 = 8;
          FUN_0050ba40(local_74,1);
          local_8 = 0xffffffff;
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_74);
          param_1[2] = 8;
          ExceptionList = local_10;
          return;
        }
        iVar3 = FUN_0061dca0();
        if ((iVar3 == 8) && (cVar1 = FUN_00430a30(), cVar1 == '\0')) {
          FUN_00430a50(1);
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          basic_string<char,std::char_traits<char>,std::allocator<char>_>
                    (local_90,"..\\Cutscenes\\CSN03EightBosses\\CSN03EightBosses.xcut");
          local_8 = 9;
          FUN_0050ba40(local_90,1);
          local_8 = 0xffffffff;
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_90);
          param_1[2] = 8;
          ExceptionList = local_10;
          return;
        }
      }
      param_1[0xf] = 0;
      local_98 = operator_new(3000);
      local_8 = 10;
      if (local_98 == (void *)0x0) {
        local_190 = 0;
      }
      else {
        local_190 = FUN_005002b0();
      }
      param_1[6] = local_190;
      break;
    case 7:
      local_28 = operator_new(0x8d4);
      local_8 = 1;
      if (local_28 == (void *)0x0) {
        local_174 = 0;
      }
      else {
        local_174 = FUN_00502820();
      }
      param_1[6] = local_174;
      break;
    case 9:
      cVar1 = FUN_00403d40();
      if (cVar1 == '\0') {
        local_b8 = operator_new(0x17d4);
        local_8 = 0xe;
        if (local_b8 == (void *)0x0) {
          local_1a0 = 0;
        }
        else {
          uVar4 = FUN_005089c0();
          local_1a0 = FUN_00503890(uVar4);
        }
        local_b4 = local_1a0;
        param_1[6] = local_1a0;
      }
      else {
        local_c0 = operator_new(0x17d4);
        local_8 = 0xf;
        if (local_c0 == (void *)0x0) {
          local_1a4 = 0;
        }
        else {
          uVar4 = FUN_005084e0();
          local_1a4 = FUN_00503890(uVar4);
        }
        local_bc = local_1a4;
        param_1[6] = local_1a4;
      }
      local_8 = 0xffffffff;
      FUN_00430a90(1);
      break;
    case 10:
      FUN_0050e610(param_1[0xf]);
      break;
    case 0xb:
      local_a0 = operator_new(0x1710);
      local_8 = 0xb;
      if (local_a0 == (void *)0x0) {
        local_194 = 0;
      }
      else {
        local_194 = FUN_004f5d50(param_1[10]);
      }
      param_1[6] = local_194;
      break;
    case 0xc:
      local_a8 = operator_new(0x3060);
      local_8 = 0xc;
      if (local_a8 == (void *)0x0) {
        local_198 = 0;
      }
      else {
        local_198 = FUN_00504ce0(param_1[10],0);
      }
      param_1[6] = local_198;
      break;
    case 0xd:
      local_b0 = operator_new(0x3060);
      local_8 = 0xd;
      if (local_b0 == (void *)0x0) {
        local_19c = 0;
      }
      else {
        local_19c = FUN_00504ce0(param_1[10],1);
      }
      param_1[6] = local_19c;
      break;
    case 0x10:
      *(undefined1 *)((int)param_1 + 0x5d) = 1;
      break;
    case 0x11:
      local_c8 = operator_new(0x1038);
      local_8 = 0x10;
      if (local_c8 == (void *)0x0) {
        local_1a8 = 0;
      }
      else {
        local_1a8 = FUN_004fd610(param_1[10]);
      }
      param_1[6] = local_1a8;
      break;
    case 0x12:
      local_d0 = operator_new(0xd30);
      local_8 = 0x11;
      if (local_d0 == (void *)0x0) {
        local_1ac = 0;
      }
      else {
        local_1ac = FUN_00616ef0(param_1[10]);
      }
      param_1[6] = local_1ac;
      break;
    case 0x15:
      local_d8 = operator_new(0xd30);
      local_8 = 0x12;
      if (local_d8 == (void *)0x0) {
        local_1b0 = 0;
      }
      else {
        local_1b0 = FUN_00615a40(param_1[10]);
      }
      param_1[6] = local_1b0;
      break;
    case 0x18:
      local_e0 = operator_new(0x1010);
      local_8 = 0x13;
      if (local_e0 == (void *)0x0) {
        local_1b4 = 0;
      }
      else {
        local_1b4 = FUN_006141c0(param_1[10]);
      }
      param_1[6] = local_1b4;
      break;
    case 0x1b:
      local_110 = operator_new(0xa50);
      local_8 = 0x19;
      if (local_110 == (void *)0x0) {
        local_1cc = 0;
      }
      else {
        local_1cc = FUN_005dad70(param_1[10]);
      }
      param_1[6] = local_1cc;
      break;
    case 0x1c:
      local_e8 = operator_new(0xae8);
      local_8 = 0x14;
      if (local_e8 == (void *)0x0) {
        local_1b8 = 0;
      }
      else {
        local_1b8 = FUN_005b8f50(param_1[10]);
      }
      param_1[6] = local_1b8;
      break;
    case 0x1e:
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::clear
                ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x10)
                );
      *(undefined1 *)(param_1 + 0x18) = 1;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (local_12c,"..\\Cutscenes\\CSN07Credits\\CSNMainCredits.xcut");
      local_8 = 0x1a;
      FUN_0050ba40(local_12c,1);
      local_8 = 0xffffffff;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_12c);
      param_1[2] = 4;
      param_1[0xc] = 0x1b;
      param_1[0xf] = 0;
      break;
    case 0x1f:
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::clear
                ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x10)
                );
      *(undefined1 *)(param_1 + 0x18) = 1;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (local_148,"..\\Cutscenes\\CSN07Credits\\CSNSpecialCredits.xcut");
      local_8 = 0x1b;
      FUN_0050ba40(local_148,1);
      local_8 = 0xffffffff;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_148);
      param_1[2] = 4;
      param_1[0xc] = 0x1b;
      param_1[0xf] = 0;
      break;
    case 0x20:
      local_f0 = operator_new(0xa50);
      local_8 = 0x15;
      if (local_f0 == (void *)0x0) {
        local_1bc = 0;
      }
      else {
        local_1bc = FUN_005d99c0(param_1[10]);
      }
      param_1[6] = local_1bc;
      break;
    case 0x21:
      local_f8 = operator_new(0xa50);
      local_8 = 0x16;
      if (local_f8 == (void *)0x0) {
        local_1c0 = 0;
      }
      else {
        local_1c0 = FUN_005d8080(param_1[10]);
      }
      param_1[6] = local_1c0;
      break;
    case 0x22:
      local_100 = operator_new(0xa50);
      local_8 = 0x17;
      if (local_100 == (void *)0x0) {
        local_1c4 = 0;
      }
      else {
        local_1c4 = FUN_005d66d0(param_1[10]);
      }
      param_1[6] = local_1c4;
      break;
    case 0x23:
      local_108 = operator_new(0xa50);
      local_8 = 0x18;
      if (local_108 == (void *)0x0) {
        local_1c8 = 0;
      }
      else {
        local_1c8 = FUN_005d4fb0(param_1[10]);
      }
      param_1[6] = local_1c8;
      break;
    case 0x41:
      FUN_00402790(param_1[0xd]);
      break;
    case 0x46:
      pvVar5 = operator_new(0xa50);
      local_8 = 0x1c;
      if (pvVar5 == (void *)0x0) {
        local_1d0 = 0;
      }
      else {
        local_1d0 = FUN_006190e0(param_1[10]);
      }
      param_1[6] = local_1d0;
      break;
    case 0x47:
      cVar1 = FUN_004033e0();
      if ((cVar1 == '\0') && (cVar1 = FUN_00403400(), cVar1 == '\0')) {
        pvVar5 = operator_new(0xa50);
        local_8 = 0x1e;
        if (pvVar5 == (void *)0x0) {
          local_1d8 = 0;
        }
        else {
          local_1d8 = FUN_004feb80(param_1[10]);
        }
        param_1[6] = local_1d8;
      }
      else {
        pvVar5 = operator_new(0xde8);
        local_8 = 0x1d;
        if (pvVar5 == (void *)0x0) {
          local_1d4 = 0;
        }
        else {
          local_1d4 = FUN_006182b0(param_1[10]);
        }
        param_1[6] = local_1d4;
      }
    }
    if (param_1[6] != 0) {
      param_1[10] = param_2;
    }
  }
  ExceptionList = local_10;
  return;
}

