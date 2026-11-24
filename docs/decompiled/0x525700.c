// FUN_00525700 @ 0x525700

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00525700(undefined4 *param_1)

{
  undefined4 uVar1;
  char *pcVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_3f8 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_3f6 [14];
  undefined4 uStack_3e8;
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
  puStack_c = &LAB_0063f572;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34,"WilyStageFiveLoop");
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,"WilyStageFiveIntro");
  local_8._0_1_ = 1;
  uStack_3e8 = 0x525768;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  *param_1 = LevelWilyFive::vftable;
  param_1[0x50] = 0;
  *(undefined1 *)(param_1 + 0x51) = 0;
  param_1[0x52] = 0;
  *(undefined1 *)(param_1 + 0x53) = 0;
  FUN_00621d10();
  local_8._0_1_ = 5;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[0x3f] = 0;
  param_1[0x40] = 0;
  param_1[0x41] = 0;
  param_1[0x42] = 0;
  param_1[0x43] = 0;
  param_1[0x44] = 0;
  param_1[0x45] = 0;
  param_1[0x46] = 0;
  param_1[0x47] = 0;
  param_1[0x48] = 0;
  param_1[0x49] = 0;
  param_1[0x4a] = 0;
  param_1[0x4b] = 0;
  param_1[0x4c] = 0;
  param_1[0x4d] = 0;
  param_1[0x4e] = 0;
  param_1[0x4f] = 0;
  param_1[2] = 0xe;
  param_1[1] = 0;
  local_54 = abStack_3f8;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_3f8,"Environment");
  uVar1 = FUN_00443e30();
  param_1[5] = uVar1;
  local_58 = abStack_3f8;
  FUN_00522dc0(abStack_3f8);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_00522d80();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_3e8 = 0x5259de;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_78,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_5c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  uStack_3e8 = 0x525a45;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_98,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_7c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
  uStack_3e8 = 0x525aae;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_9c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b8);
  uStack_3e8 = 0x525b1b;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_bc = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d8);
  uStack_3e8 = 0x525b88;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_dc = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_f8);
  uStack_3e8 = 0x525bf4;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_118,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_fc = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_118);
  uStack_3e8 = 0x525c61;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_138,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_11c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_138);
  uStack_3e8 = 0x525cce;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_158,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_13c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158);
  uStack_3e8 = 0x525d3a;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_178,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xe;
  local_15c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_178);
  uStack_3e8 = 0x525da7;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_198,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xf;
  local_17c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_198);
  uStack_3e8 = 0x525e14;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x10;
  local_19c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1b8);
  uStack_3e8 = 0x525e80;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x11;
  local_1bc = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1d8);
  uStack_3e8 = 0x525eed;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_1dc = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1f8);
  uStack_3e8 = 0x525f5a;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_218,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x13;
  local_1fc = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_218);
  uStack_3e8 = 0x525fc6;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_238,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_21c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_238);
  uStack_3e8 = 0x526033;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_258,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x15;
  local_23c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_258);
  uStack_3e8 = 0x5260a0;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_278,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_25c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_278);
  uStack_3e8 = 0x52610c;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_298,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x17;
  local_27c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_298);
  uStack_3e8 = 0x526179;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x18;
  local_29c = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2b8);
  uStack_3e8 = 0x5261e6;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x19;
  local_2bc = abStack_3f8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_3f8 + -4),pcVar2);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,5);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2d8);
  FUN_005391f0();
  FUN_0043bc00();
  uStack_3e8 = 0x52626a;
  FUN_00522b10();
  FUN_00403ef0();
  FUN_00403ef0();
  FUN_00523bf0();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

