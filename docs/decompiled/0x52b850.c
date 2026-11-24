// FUN_0052b850 @ 0x52b850

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_0052b850(undefined4 *param_1)

{
  char cVar1;
  undefined4 uVar2;
  char *pcVar3;
  undefined4 *puVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_6e0 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_6de [14];
  undefined4 uStack_6d0;
  char *local_504;
  char *local_500;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_4d8 [28];
  undefined1 *local_4bc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_4b8 [28];
  undefined1 *local_49c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_498 [28];
  undefined1 *local_47c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_478 [28];
  undefined1 *local_45c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_458 [28];
  undefined1 *local_43c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_438 [28];
  undefined1 *local_41c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_418 [28];
  undefined1 *local_3fc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3f8 [28];
  undefined1 *local_3dc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3d8 [28];
  undefined1 *local_3bc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3b8 [28];
  undefined1 *local_39c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_398 [28];
  undefined1 *local_37c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_378 [28];
  undefined1 *local_35c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_358 [28];
  undefined1 *local_33c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_338 [28];
  undefined1 *local_31c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_318 [28];
  undefined1 *local_2fc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2f8 [28];
  undefined1 *local_2dc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2d8 [28];
  undefined1 *local_2bc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2b8 [28];
  undefined1 *local_29c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_298 [28];
  undefined1 *local_27c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_278 [28];
  undefined1 *local_25c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_258 [28];
  undefined1 *local_23c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_238 [28];
  undefined1 *local_21c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_218 [28];
  undefined1 *local_1fc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_1f8 [28];
  undefined1 *local_1dc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_1d8 [28];
  undefined1 *local_1bc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_1b8 [28];
  undefined1 *local_19c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_198 [28];
  undefined1 *local_17c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_178 [28];
  undefined1 *local_15c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_158 [28];
  undefined1 *local_13c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_138 [28];
  undefined1 *local_11c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_118 [28];
  undefined1 *local_fc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_f8 [28];
  undefined1 *local_dc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_d8 [28];
  undefined1 *local_bc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_b8 [28];
  undefined1 *local_9c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_98 [28];
  undefined1 *local_7c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_78 [28];
  undefined1 *local_5c;
  undefined1 *local_58;
  undefined1 *local_54;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_50 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_34 [28];
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063f89a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  cVar1 = FUN_004042e0();
  if (cVar1 == '\0') {
    local_500 = "RainbowManStageLoop";
  }
  else {
    local_500 = "RainbowManStageLoopAlt";
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34,local_500);
  local_8 = 0;
  cVar1 = FUN_004042e0();
  if (cVar1 == '\0') {
    local_504 = "";
  }
  else {
    local_504 = "";
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,local_504);
  local_8._0_1_ = 1;
  uStack_6d0 = 0x52b90c;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  *param_1 = LevelRainbowMan::vftable;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[0x3f] = 0;
  param_1[2] = 4;
  param_1[1] = 0;
  local_54 = abStack_6e0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_6e0,"Environment");
  uVar2 = FUN_00443e30();
  param_1[5] = uVar2;
  local_58 = abStack_6e0;
  FUN_005299a0(abStack_6e0);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_00529970();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_6d0 = 0x52ba36;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_78,PTR_DAT_006c1fd0);
  local_8._0_1_ = 5;
  local_5c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  uStack_6d0 = 0x52ba9d;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_98,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_7c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
  uStack_6d0 = 0x52bb07;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_9c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b8);
  uStack_6d0 = 0x52bb73;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_bc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d8);
  uStack_6d0 = 0x52bbe0;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_dc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_f8);
  uStack_6d0 = 0x52bc4d;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_118,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_fc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_118);
  uStack_6d0 = 0x52bcb9;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_138,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_11c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_138);
  uStack_6d0 = 0x52bd26;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_158,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_13c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158);
  uStack_6d0 = 0x52bd93;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_178,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_15c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_178);
  uStack_6d0 = 0x52bdff;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_198,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xe;
  local_17c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_198);
  uStack_6d0 = 0x52be6c;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xf;
  local_19c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1b8);
  uStack_6d0 = 0x52bed9;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x10;
  local_1bc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1d8);
  uStack_6d0 = 0x52bf45;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x11;
  local_1dc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1f8);
  uStack_6d0 = 0x52bfb2;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_218,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_1fc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_218);
  uStack_6d0 = 0x52c01f;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_238,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x13;
  local_21c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_238);
  uStack_6d0 = 0x52c08b;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_258,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_23c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_258);
  uStack_6d0 = 0x52c0f8;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_278,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x15;
  local_25c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_278);
  uStack_6d0 = 0x52c165;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_298,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_27c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_298);
  uStack_6d0 = 0x52c1d1;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x17;
  local_29c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2b8);
  uStack_6d0 = 0x52c23e;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x18;
  local_2bc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2d8);
  uStack_6d0 = 0x52c2ab;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x19;
  local_2dc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2f8);
  uStack_6d0 = 0x52c317;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_318,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1a;
  local_2fc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_318);
  uStack_6d0 = 0x52c384;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_338,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1b;
  local_31c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_338);
  uStack_6d0 = 0x52c3f1;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_358,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1c;
  local_33c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_358);
  uStack_6d0 = 0x52c45d;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_378,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1d;
  local_35c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_378);
  uStack_6d0 = 0x52c4ca;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_398,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1e;
  local_37c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_398);
  uStack_6d0 = 0x52c537;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1f;
  local_39c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3b8);
  uStack_6d0 = 0x52c5a3;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x20;
  local_3bc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3d8);
  uStack_6d0 = 0x52c610;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x21;
  local_3dc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3f8);
  uStack_6d0 = 0x52c67d;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_418,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x22;
  local_3fc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_418);
  uStack_6d0 = 0x52c6e9;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_438,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x23;
  local_41c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_438);
  uStack_6d0 = 0x52c756;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_458,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x24;
  local_43c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_458);
  uStack_6d0 = 0x52c7c3;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_478,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x25;
  local_45c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_478);
  uStack_6d0 = 0x52c82f;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_498,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x26;
  local_47c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_498);
  uStack_6d0 = 0x52c89c;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_4b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x27;
  local_49c = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_4b8);
  uStack_6d0 = 0x52c909;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_4d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x28;
  local_4bc = abStack_6e0 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_6e0 + -4),pcVar3);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_4d8);
  puVar4 = (undefined4 *)FUN_00472660();
  local_18 = *puVar4;
  local_14 = puVar4[1];
  while( true ) {
    FUN_0046df60();
    cVar1 = FUN_00426920();
    if (cVar1 == '\0') break;
    uStack_6d0 = 0x52c9b5;
    FUN_0043bc20();
    FUN_004bdc80();
    FUN_00621180();
    uStack_6d0 = 0x52c9d6;
    FUN_0042f8a0();
  }
  FUN_0052b040();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

