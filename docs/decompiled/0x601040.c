// FUN_00601040 @ 0x601040

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00601040(undefined4 *param_1)

{
  char cVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  char *pcVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_4b8 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_4b6 [14];
  undefined4 uStack_4a8;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_368 [28];
  undefined1 *local_34c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_348 [28];
  undefined1 *local_32c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_328 [28];
  undefined1 *local_30c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_308 [28];
  undefined1 *local_2ec;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2e8 [28];
  undefined1 *local_2cc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c8 [28];
  undefined1 *local_2ac;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2a8 [28];
  undefined1 *local_28c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_288 [28];
  undefined1 *local_26c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_268 [28];
  undefined1 *local_24c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_248 [28];
  undefined1 *local_22c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_228 [28];
  undefined1 *local_20c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_208 [28];
  undefined1 *local_1ec;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_1e8 [28];
  undefined1 *local_1cc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_1c8 [28];
  undefined1 *local_1ac;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_1a8 [28];
  undefined1 *local_18c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_188 [28];
  undefined1 *local_16c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_168 [28];
  undefined1 *local_14c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_148 [28];
  undefined1 *local_12c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_128 [28];
  undefined1 *local_10c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_108 [28];
  undefined1 *local_ec;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_e8 [28];
  undefined1 *local_cc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_c8 [28];
  undefined1 *local_ac;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_a8 [28];
  undefined1 *local_8c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_88 [28];
  undefined1 *local_6c;
  undefined1 *local_68;
  undefined1 *local_64;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_58 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3c [44];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063d4fe;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3c,"EndlessBossAttackLoop");
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_58,"EndlessBossAttackIntro")
  ;
  local_8._0_1_ = 1;
  uStack_4a8 = 0x6010a8;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_58);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3c);
  *param_1 = LevelEndlessBoss::vftable;
  *(undefined1 *)(param_1 + 0x3d) = 0;
  *(undefined1 *)(param_1 + 0x42) = 0;
  *(undefined1 *)(param_1 + 0x44) = 1;
  *(undefined1 *)((int)param_1 + 0x111) = 1;
  FUN_00403ff0();
  local_8 = CONCAT31(local_8._1_3_,5);
  FUN_004304c0();
  FUN_00430490();
  while( true ) {
    FUN_004d5630();
    cVar1 = FUN_0046bcb0();
    if (cVar1 == '\0') break;
    puVar2 = (undefined4 *)FUN_00467d30();
    *puVar2 = 0;
    FUN_00497210();
  }
  param_1[0x3e] = 0;
  param_1[0x3f] = 0;
  param_1[0x40] = 0;
  param_1[0x41] = 0;
  param_1[2] = 0x11;
  param_1[1] = 0;
  local_64 = abStack_4b8;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_4b8,"Environment");
  uVar3 = FUN_00443e30();
  param_1[5] = uVar3;
  local_68 = abStack_4b8;
  FUN_005ffbf0(abStack_4b8);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_005ffbc0();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_4a8 = 0x601296;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_88,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_6c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_88);
  uStack_4a8 = 0x601300;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_8c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_a8);
  uStack_4a8 = 0x60136c;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_ac = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_c8);
  uStack_4a8 = 0x6013d9;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_cc = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_e8);
  uStack_4a8 = 0x601446;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_108,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_ec = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_108);
  uStack_4a8 = 0x6014b2;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_128,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_10c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_128);
  uStack_4a8 = 0x60151f;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_148,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_12c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_148);
  uStack_4a8 = 0x60158c;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_168,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_14c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_168);
  uStack_4a8 = 0x6015f8;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_188,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xe;
  local_16c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_188);
  uStack_4a8 = 0x601665;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xf;
  local_18c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1a8);
  uStack_4a8 = 0x6016d2;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x10;
  local_1ac = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1c8);
  uStack_4a8 = 0x60173e;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x11;
  local_1cc = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1e8);
  uStack_4a8 = 0x6017ab;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_208,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_1ec = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_208);
  uStack_4a8 = 0x601818;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_228,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x13;
  local_20c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_228);
  uStack_4a8 = 0x601884;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_248,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_22c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_248);
  uStack_4a8 = 0x6018f1;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_268,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x15;
  local_24c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_268);
  uStack_4a8 = 0x60195e;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_288,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_26c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_288);
  uStack_4a8 = 0x6019ca;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x17;
  local_28c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2a8);
  uStack_4a8 = 0x601a37;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x18;
  local_2ac = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c8);
  uStack_4a8 = 0x601aa4;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x19;
  local_2cc = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2e8);
  uStack_4a8 = 0x601b10;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_308,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1a;
  local_2ec = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_308);
  uStack_4a8 = 0x601b7d;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_328,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1b;
  local_30c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_328);
  uStack_4a8 = 0x601bea;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_348,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1c;
  local_32c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_348);
  uStack_4a8 = 0x601c56;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_368,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1d;
  local_34c = abStack_4b8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_4b8 + -4),pcVar4);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,5);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_368);
  FUN_005391f0();
  FUN_0043bc00();
  uStack_4a8 = 0x601cda;
  FUN_005ff8e0();
  FUN_00403ef0();
  FUN_00403ef0();
  FUN_00600c50();
  FUN_004ac790();
  FUN_00500e90();
  FUN_0061e1e0();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  FUN_0061dc50();
  cVar1 = FUN_00403d40();
  if (cVar1 == '\0') {
    FUN_0061dc50();
  }
  else {
    FUN_0061dc20();
  }
  cVar1 = FUN_00403f70();
  if (cVar1 == '\x01') {
    cVar1 = FUN_00403d40();
    if (cVar1 == '\0') {
      FUN_0061dc50();
      goto LAB_00601e5e;
    }
  }
  cVar1 = FUN_00403f70();
  if (cVar1 == '\x01') {
    cVar1 = FUN_00403d40();
    if (cVar1 != '\0') {
      FUN_0061dc20();
      goto LAB_00601e5e;
    }
  }
  cVar1 = FUN_00403f70();
  if (cVar1 == '\0') {
    FUN_0061dc20();
  }
LAB_00601e5e:
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

