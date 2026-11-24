// FUN_00518750 @ 0x518750

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00518750(undefined4 *param_1)

{
  undefined4 uVar1;
  char *pcVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_424 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_422 [14];
  undefined4 uStack_414;
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
  puStack_c = &LAB_0063ef99;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34,"WilyStageTwoLoop");
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,"");
  local_8._0_1_ = 1;
  uStack_414 = 0x5187b8;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  *param_1 = LevelWilyTwo::vftable;
  *(undefined1 *)(param_1 + 0x3f) = 0;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[2] = 0xb;
  param_1[1] = 0;
  local_54 = abStack_424;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_424,"Environment");
  uVar1 = FUN_00443e30();
  param_1[5] = uVar1;
  local_58 = abStack_424;
  FUN_005173a0(abStack_424);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_00517340();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_414 = 0x5188e0;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_78,PTR_DAT_006c1fd0);
  local_8._0_1_ = 5;
  local_5c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  uStack_414 = 0x518946;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_98,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_7c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
  uStack_414 = 0x5189b0;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_9c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b8);
  uStack_414 = 0x518a1d;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_bc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d8);
  uStack_414 = 0x518a89;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_dc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_f8);
  uStack_414 = 0x518af6;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_118,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_fc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_118);
  uStack_414 = 0x518b63;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_138,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_11c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_138);
  uStack_414 = 0x518bcf;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_158,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_13c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158);
  uStack_414 = 0x518c3c;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_178,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_15c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_178);
  uStack_414 = 0x518ca9;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_198,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xe;
  local_17c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_198);
  uStack_414 = 0x518d15;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xf;
  local_19c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1b8);
  uStack_414 = 0x518d82;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x10;
  local_1bc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1d8);
  uStack_414 = 0x518def;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x11;
  local_1dc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1f8);
  uStack_414 = 0x518e5b;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_218,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_1fc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_218);
  uStack_414 = 0x518ec8;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_238,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x13;
  local_21c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_238);
  uStack_414 = 0x518f35;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_258,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_23c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_258);
  uStack_414 = 0x518fa1;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_278,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x15;
  local_25c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_278);
  uStack_414 = 0x51900e;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_298,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_27c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_298);
  uStack_414 = 0x51907b;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x17;
  local_29c = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2b8);
  uStack_414 = 0x5190e7;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x18;
  local_2bc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2d8);
  uStack_414 = 0x519154;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x19;
  local_2dc = abStack_424 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_424 + -4),pcVar2);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2f8);
  FUN_005391f0();
  FUN_0043bc00();
  uStack_414 = 0x5191d8;
  FUN_00517090();
  FUN_00403ef0();
  FUN_00403ef0();
  FUN_00518070();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

