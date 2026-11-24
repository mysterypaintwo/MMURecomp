// FUN_005341c0 @ 0x5341c0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_005341c0(undefined4 *param_1)

{
  undefined4 uVar1;
  char *pcVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_450 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_44e [14];
  undefined4 uStack_440;
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
  puStack_c = &LAB_0063fcd2;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34,"JetManStageLoop");
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,"");
  local_8._0_1_ = 1;
  uStack_440 = 0x534228;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  *param_1 = LevelJetMan::vftable;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  *(undefined1 *)(param_1 + 0x3f) = 0;
  param_1[2] = 8;
  param_1[1] = 0;
  local_54 = abStack_450;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_450,"Environment");
  uVar1 = FUN_00443e30();
  param_1[5] = uVar1;
  local_58 = abStack_450;
  FUN_00532bf0(abStack_450);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_00532bd0();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_440 = 0x534350;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_78,PTR_DAT_006c1fd0);
  local_8._0_1_ = 5;
  local_5c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  uStack_440 = 0x5343b6;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_98,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_7c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
  uStack_440 = 0x534420;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_9c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b8);
  uStack_440 = 0x53448d;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_bc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d8);
  uStack_440 = 0x5344f9;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_dc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_f8);
  uStack_440 = 0x534566;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_118,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_fc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_118);
  uStack_440 = 0x5345d3;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_138,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_11c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_138);
  uStack_440 = 0x53463f;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_158,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_13c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158);
  uStack_440 = 0x5346ac;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_178,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_15c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_178);
  uStack_440 = 0x534719;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_198,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xe;
  local_17c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_198);
  uStack_440 = 0x534785;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xf;
  local_19c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1b8);
  uStack_440 = 0x5347f2;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x10;
  local_1bc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1d8);
  uStack_440 = 0x53485f;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x11;
  local_1dc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1f8);
  uStack_440 = 0x5348cb;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_218,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_1fc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_218);
  uStack_440 = 0x534938;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_238,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x13;
  local_21c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_238);
  uStack_440 = 0x5349a5;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_258,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_23c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_258);
  uStack_440 = 0x534a11;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_278,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x15;
  local_25c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_278);
  uStack_440 = 0x534a7e;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_298,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_27c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_298);
  uStack_440 = 0x534aeb;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x17;
  local_29c = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2b8);
  uStack_440 = 0x534b57;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x18;
  local_2bc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2d8);
  uStack_440 = 0x534bc4;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x19;
  local_2dc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2f8);
  uStack_440 = 0x534c31;
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_318,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1a;
  local_2fc = abStack_450 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_450 + -4),pcVar2);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_318);
  FUN_005391f0();
  FUN_0043bc00();
  uStack_440 = 0x534cb5;
  FUN_00532900();
  FUN_00403ef0();
  FUN_00403ef0();
  FUN_00533950();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

