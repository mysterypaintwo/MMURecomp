// FUN_00515dc0 @ 0x515dc0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00515dc0(undefined4 *param_1)

{
  undefined4 uVar1;
  char *pcVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_608 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_606 [14];
  undefined4 uStack_5f8;
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
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_34 [36];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063ee65;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34,"YokuManStageLoop");
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,"YokuManStageIntro");
  local_8._0_1_ = 1;
  uStack_5f8 = 0x515e28;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  *param_1 = LevelYokuMan::vftable;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[2] = 9;
  param_1[1] = 0;
  local_54 = abStack_608;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_608,"Environment");
  uVar1 = FUN_00443e30();
  param_1[5] = uVar1;
  local_58 = abStack_608;
  FUN_00513990(abStack_608);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_00513930();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_5f8 = 0x515f43;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_78,PTR_DAT_006c1fd0);
  local_8._0_1_ = 5;
  local_5c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  uStack_5f8 = 0x515faa;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_98,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_7c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
  uStack_5f8 = 0x516013;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_9c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b8);
  uStack_5f8 = 0x516080;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_bc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d8);
  uStack_5f8 = 0x5160ed;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_dc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_f8);
  uStack_5f8 = 0x516159;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_118,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_fc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_118);
  uStack_5f8 = 0x5161c6;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_138,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_11c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_138);
  uStack_5f8 = 0x516233;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_158,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_13c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158);
  uStack_5f8 = 0x51629f;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_178,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_15c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_178);
  uStack_5f8 = 0x51630c;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_198,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xe;
  local_17c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_198);
  uStack_5f8 = 0x516379;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xf;
  local_19c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1b8);
  uStack_5f8 = 0x5163e5;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x10;
  local_1bc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1d8);
  uStack_5f8 = 0x516452;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x11;
  local_1dc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1f8);
  uStack_5f8 = 0x5164bf;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_218,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_1fc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_218);
  uStack_5f8 = 0x51652b;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_238,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x13;
  local_21c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_238);
  uStack_5f8 = 0x516598;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_258,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_23c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_258);
  uStack_5f8 = 0x516605;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_278,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x15;
  local_25c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_278);
  uStack_5f8 = 0x516671;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_298,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_27c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_298);
  uStack_5f8 = 0x5166de;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x17;
  local_29c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2b8);
  uStack_5f8 = 0x51674b;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x18;
  local_2bc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2d8);
  uStack_5f8 = 0x5167b7;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x19;
  local_2dc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2f8);
  uStack_5f8 = 0x516824;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_318,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1a;
  local_2fc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_318);
  uStack_5f8 = 0x516891;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_338,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1b;
  local_31c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_338);
  uStack_5f8 = 0x5168fd;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_358,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1c;
  local_33c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_358);
  uStack_5f8 = 0x51696a;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_378,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1d;
  local_35c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_378);
  uStack_5f8 = 0x5169d7;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_398,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1e;
  local_37c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_398);
  uStack_5f8 = 0x516a43;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1f;
  local_39c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3b8);
  uStack_5f8 = 0x516ab0;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x20;
  local_3bc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3d8);
  uStack_5f8 = 0x516b1d;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x21;
  local_3dc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3f8);
  uStack_5f8 = 0x516b89;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_418,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x22;
  local_3fc = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_418);
  uStack_5f8 = 0x516bf6;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_438,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x23;
  local_41c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_438);
  uStack_5f8 = 0x516c63;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_458,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x24;
  local_43c = abStack_608 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_608 + -4),pcVar2);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_458);
  FUN_005391f0();
  FUN_0043bc00();
  uStack_5f8 = 0x516ce7;
  FUN_00513520();
  FUN_00403ef0();
  FUN_00403ef0();
  FUN_00514da0();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

