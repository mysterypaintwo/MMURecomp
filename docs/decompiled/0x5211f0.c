// FUN_005211f0 @ 0x5211f0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_005211f0(undefined4 *param_1)

{
  char cVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  char *pcVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_750 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_74e [14];
  undefined4 uStack_740;
  undefined1 local_54e;
  undefined1 local_54d;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_548 [28];
  undefined1 *local_52c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_528 [28];
  undefined1 *local_50c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_508 [28];
  undefined1 *local_4ec;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_4e8 [28];
  undefined1 *local_4cc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_4c8 [28];
  undefined1 *local_4ac;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_4a8 [28];
  undefined1 *local_48c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_488 [28];
  undefined1 *local_46c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_468 [28];
  undefined1 *local_44c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_448 [28];
  undefined1 *local_42c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_428 [28];
  undefined1 *local_40c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_408 [28];
  undefined1 *local_3ec;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3e8 [28];
  undefined1 *local_3cc;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3c8 [28];
  undefined1 *local_3ac;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_3a8 [28];
  undefined1 *local_38c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_388 [28];
  undefined1 *local_36c;
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
  puStack_c = &LAB_0063f439;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3c,"WilyStageFourLoop");
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_58,"WilyStageFourIntro");
  local_8._0_1_ = 1;
  uStack_740 = 0x521258;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_58);
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3c);
  *param_1 = LevelWilyFour::vftable;
  *(undefined1 *)(param_1 + 0x41) = 0;
  *(undefined1 *)((int)param_1 + 0x105) = 0;
  cVar1 = FUN_00425e60();
  if (cVar1 == '\0') {
    cVar1 = FUN_00403d40();
    if (cVar1 != '\0') goto LAB_005212c5;
    local_54d = 1;
  }
  else {
LAB_005212c5:
    local_54d = 0;
  }
  *(undefined1 *)(param_1 + 0x43) = local_54d;
  cVar1 = FUN_00425e60();
  if (cVar1 == '\0') {
    cVar1 = FUN_00403d40();
    if (cVar1 == '\0') {
      local_54e = 1;
      goto LAB_00521312;
    }
  }
  local_54e = 0;
LAB_00521312:
  *(undefined1 *)((int)param_1 + 0x10d) = local_54e;
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
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[0x3f] = 0;
  param_1[0x40] = 0;
  param_1[2] = 0xd;
  param_1[1] = 0;
  local_64 = abStack_750;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_750,"Environment");
  uVar3 = FUN_00443e30();
  param_1[5] = uVar3;
  local_68 = abStack_750;
  FUN_0051e6c0(abStack_750);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_0051e660();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_740 = 0x5214b8;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_88,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_6c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_88);
  uStack_740 = 0x521522;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_8c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_a8);
  uStack_740 = 0x52158e;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_ac = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_c8);
  uStack_740 = 0x5215fb;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_cc = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_e8);
  uStack_740 = 0x521668;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_108,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_ec = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_108);
  uStack_740 = 0x5216d4;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_128,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_10c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_128);
  uStack_740 = 0x521741;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_148,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_12c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_148);
  uStack_740 = 0x5217ae;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_168,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_14c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_168);
  uStack_740 = 0x52181a;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_188,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xe;
  local_16c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_188);
  uStack_740 = 0x521887;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xf;
  local_18c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1a8);
  uStack_740 = 0x5218f4;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x10;
  local_1ac = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1c8);
  uStack_740 = 0x521960;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_1e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x11;
  local_1cc = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1e8);
  uStack_740 = 0x5219cd;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_208,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_1ec = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_208);
  uStack_740 = 0x521a3a;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_228,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x13;
  local_20c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_228);
  uStack_740 = 0x521aa6;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_248,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_22c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_248);
  uStack_740 = 0x521b13;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_268,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x15;
  local_24c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_268);
  uStack_740 = 0x521b80;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_288,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_26c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_288);
  uStack_740 = 0x521bec;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x17;
  local_28c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2a8);
  uStack_740 = 0x521c59;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x18;
  local_2ac = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c8);
  uStack_740 = 0x521cc6;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_2e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x19;
  local_2cc = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2e8);
  uStack_740 = 0x521d32;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_308,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1a;
  local_2ec = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_308);
  uStack_740 = 0x521d9f;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_328,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1b;
  local_30c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_328);
  uStack_740 = 0x521e0c;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_348,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1c;
  local_32c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_348);
  uStack_740 = 0x521e78;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_368,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1d;
  local_34c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_368);
  uStack_740 = 0x521ee5;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_388,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1e;
  local_36c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_388);
  uStack_740 = 0x521f52;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x1f;
  local_38c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3a8);
  uStack_740 = 0x521fbe;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x20;
  local_3ac = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3c8);
  uStack_740 = 0x52202b;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_3e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x21;
  local_3cc = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_3e8);
  uStack_740 = 0x522098;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_408,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x22;
  local_3ec = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_408);
  uStack_740 = 0x522104;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_428,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x23;
  local_40c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_428);
  uStack_740 = 0x522171;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_448,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x24;
  local_42c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_448);
  uStack_740 = 0x5221de;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_468,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x25;
  local_44c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_468);
  uStack_740 = 0x52224a;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_488,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x26;
  local_46c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_488);
  uStack_740 = 0x5222b7;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_4a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x27;
  local_48c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_4a8);
  uStack_740 = 0x522324;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_4c8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x28;
  local_4ac = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_4c8);
  uStack_740 = 0x522390;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_4e8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x29;
  local_4cc = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_4e8);
  uStack_740 = 0x5223fd;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_508,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x2a;
  local_4ec = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_508);
  uStack_740 = 0x52246a;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_528,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x2b;
  local_50c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8._0_1_ = 5;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_528);
  uStack_740 = 0x5224d6;
  pcVar4 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_548,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x2c;
  local_52c = abStack_750 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_750 + -4),pcVar4);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,5);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_548);
  FUN_005391f0();
  FUN_0043bc00();
  uStack_740 = 0x52255a;
  FUN_0051e180();
  FUN_00403ef0();
  FUN_00403ef0();
  FUN_0051ffe0();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

