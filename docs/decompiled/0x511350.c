// FUN_00511350 @ 0x511350

void __fastcall FUN_00511350(int *param_1)

{
  char cVar1;
  uint uVar2;
  int local_cc;
  undefined4 local_c8;
  int local_c4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_bc [28];
  undefined4 *local_a0;
  undefined4 *local_9c;
  undefined4 local_98;
  void *local_94;
  int local_90;
  void *local_8c;
  int local_88;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_84 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_68 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_4c [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_30 [31];
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_006424d0;
  local_10 = ExceptionList;
  uVar2 = DAT_006c1e94 ^ (uint)(local_30 + 0x2c);
  ExceptionList = &local_10;
  cVar1 = FUN_00441d90(uVar2);
  if (cVar1 == '\0') {
    cVar1 = FUN_00441d90(uVar2);
    if (cVar1 == '\0') {
      cVar1 = FUN_00441d90(uVar2);
      if (cVar1 == '\0') {
        cVar1 = FUN_00441d90(uVar2);
        if (cVar1 == '\0') {
          cVar1 = FUN_00441d90(uVar2);
          if (cVar1 == '\0') {
            cVar1 = FUN_00441d90();
            if (cVar1 == '\0') {
              cVar1 = FUN_00441d90();
              if (cVar1 == '\0') {
                cVar1 = FUN_00441d90();
                if (cVar1 == '\0') {
                  if (*param_1 == 1) {
                    cVar1 = FUN_00441d90();
                    if (cVar1 != '\0') {
                      *(undefined1 *)((int)param_1 + 0x5f) = 1;
                      *param_1 = 4;
                      local_8c = operator_new(0x347c);
                      local_8 = 4;
                      if (local_8c == (void *)0x0) {
                        local_c4 = 0;
                      }
                      else {
                        local_c4 = FUN_004f9410();
                      }
                      local_88 = local_c4;
                      local_8 = 0xffffffff;
                      param_1[7] = local_c4;
                      FUN_00507f00(1);
                      ExceptionList = local_10;
                      return;
                    }
                    cVar1 = FUN_00441d90();
                    if (cVar1 != '\0') {
                      *(undefined1 *)((int)param_1 + 0x5f) = 1;
                      *param_1 = 4;
                      local_11 = '\0';
                      if ((0 < param_1[0xf]) && (param_1[0xf] < 10)) {
                        local_11 = FUN_0061daf0(*(undefined4 *)(&DAT_006c20f4 + param_1[0xf] * 4));
                      }
                      local_94 = operator_new(0x17d4);
                      local_8 = 5;
                      if (local_94 == (void *)0x0) {
                        local_cc = 0;
                      }
                      else {
                        if (local_11 == '\0') {
                          local_c8 = FUN_00508340();
                        }
                        else {
                          local_c8 = FUN_005081a0();
                        }
                        local_98 = local_c8;
                        local_cc = FUN_00503890(local_c8);
                      }
                      local_90 = local_cc;
                      local_8 = 0xffffffff;
                      param_1[7] = local_cc;
                      FUN_004026b0("PauseMenu",0,0);
                      FUN_00507f00(1);
                      ExceptionList = local_10;
                      return;
                    }
                    if (((char)param_1[0x17] == '\0') && (param_1[7] != 0)) {
                      local_a0 = (undefined4 *)param_1[7];
                      local_9c = local_a0;
                      if (local_a0 != (undefined4 *)0x0) {
                        (**(code **)*local_a0)(1);
                      }
                      param_1[7] = 0;
                    }
                  }
                  else {
                    FUN_004308a0();
                  }
                  if (*(char *)((int)param_1 + 0x27) != '\0') {
                    *(undefined1 *)((int)param_1 + 0x27) = 0;
                    FUN_00510c20();
                  }
                  if (*param_1 == 0x100) {
                    cVar1 = FUN_00403d40();
                    if (cVar1 == '\0') {
                      FUN_0050ba40(&DAT_006c6964,1);
                    }
                    else {
                      FUN_0050ba40(&DAT_006c6a98,1);
                    }
                    param_1[2] = 0x100;
                    cVar1 = FUN_00403d40();
                    if (cVar1 == '\0') {
                      FUN_004308c0(&DAT_006c6964,1);
                    }
                    else {
                      FUN_004308c0(&DAT_006c6a98,1);
                    }
                  }
                  else if (*param_1 == 0x200) {
                    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                    basic_string<char,std::char_traits<char>,std::allocator<char>_>
                              (local_bc,"..\\Cutscenes\\CSN01Intro\\csn01Intro.xcut");
                    local_8 = 6;
                    FUN_0050ba40(local_bc,1);
                    local_8 = 0xffffffff;
                    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_bc);
                    param_1[2] = 4;
                    param_1[0xc] = 1;
                  }
                  else if (*param_1 == 0x40) {
                    FUN_00510c80();
                  }
                  else {
                    if ((char)param_1[9] != '\0') {
                      FUN_0054a190();
                      *(undefined1 *)(param_1 + 9) = 0;
                      if (*(char *)((int)param_1 + 0x25) == '\0') {
                        (**(code **)(*(int *)param_1[8] + 0x10))();
                      }
                      else if (param_1[0xf] == 0x10) {
                        FUN_00510ba0();
                      }
                      else if (param_1[0xf] == 0x11) {
                        FUN_00510b20();
                      }
                      else if (param_1[0xf] == 0x12) {
                        FUN_00510aa0();
                      }
                      else if (param_1[0xf] == 0x13) {
                        FUN_00510a10();
                      }
                      else if ((((param_1[0xf] == 0x1e) || (param_1[0xf] == 0x1f)) ||
                               (param_1[0xf] == 0x20)) ||
                              (((param_1[0xf] == 0x21 || (param_1[0xf] == 0x22)) ||
                               ((param_1[0xf] == 0x23 || (param_1[0xf] == 0x24)))))) {
                        FUN_00510990();
                      }
                      else if (((param_1[0xf] == 0x25) || (param_1[0xf] == 0x26)) ||
                              ((param_1[0xf] == 0x27 ||
                               ((((param_1[0xf] == 0x14 || (param_1[0xf] == 0x15)) ||
                                 (param_1[0xf] == 0x16)) || (param_1[0xf] == 0x17)))))) {
                        FUN_00510910();
                      }
                      else if ((((param_1[0xf] == 0x18) || (param_1[0xf] == 0x19)) ||
                               ((param_1[0xf] == 0x1a ||
                                ((param_1[0xf] == 0x1b || (param_1[0xf] == 0x1c)))))) ||
                              (param_1[0xf] == 0x1d)) {
                        FUN_00510890();
                      }
                      else {
                        FUN_0050bb10();
                      }
                    }
                    if (((param_1[0xe] != 0) && (param_1[5] == 0)) &&
                       (cVar1 = FUN_0050be40(param_1[0xe]), cVar1 == '\0')) {
                      FUN_0050e610(param_1[0xe]);
                      param_1[0xe] = 0;
                    }
                    if (param_1[8] != 0) {
                      (**(code **)(*(int *)param_1[8] + 8))();
                    }
                    if ((param_1[8] != 0) && (*param_1 == 1)) {
                      (**(code **)(*(int *)param_1[8] + 4))();
                    }
                  }
                }
                else {
                  FUN_00507fd0(1);
                  FUN_0050ba40(&DAT_006c6a3c,1);
                }
              }
              else {
                FUN_00507fd0(1);
                FUN_0050ba40(&DAT_006c6a98,1);
              }
            }
            else {
              FUN_00507fd0(1);
              FUN_0050ba40(&DAT_006c6a00,1);
            }
          }
          else {
            FUN_00507fd0(1);
            FUN_0050ba40(&DAT_006c6964,1);
          }
        }
        else {
          FUN_00507fd0(1);
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          basic_string<char,std::char_traits<char>,std::allocator<char>_>
                    (local_84,"..\\Cutscenes\\CSN04CastleScenes\\CSN04CastleMap01.xcut");
          local_8 = 3;
          FUN_0050ba40(local_84,1);
          local_8 = 0xffffffff;
          std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
          ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_84);
        }
      }
      else {
        FUN_00507fd0(1);
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        basic_string<char,std::char_traits<char>,std::allocator<char>_>
                  (local_68,"..\\Cutscenes\\CSN03EightBosses\\TCNGETSpeedster.xcut");
        local_8 = 2;
        FUN_0050ba40(local_68,1);
        local_8 = 0xffffffff;
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
        ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_68);
      }
    }
    else {
      FUN_00507fd0(1);
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      basic_string<char,std::char_traits<char>,std::allocator<char>_>
                (local_4c,"..\\Cutscenes\\CSN02FourBosses\\CSN02FourBosses.xcut");
      local_8 = 1;
      FUN_0050ba40(local_4c,1);
      local_8 = 0xffffffff;
      std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
      ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_4c);
    }
  }
  else {
    FUN_00507fd0(1);
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>
              (local_30,"..\\Cutscenes\\CSN01Intro\\csn01Intro.xcut");
    local_8 = 0;
    FUN_0050ba40(local_30,1);
    local_8 = 0xffffffff;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_30);
  }
  ExceptionList = local_10;
  return;
}

