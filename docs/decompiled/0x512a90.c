// FUN_00512a90 @ 0x512a90

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00512a90(undefined4 *param_1)

{
  char cVar1;
  undefined4 uVar2;
  char *pcVar3;
  undefined4 *puVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_234 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_232 [14];
  undefined4 uStack_224;
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
  puStack_c = &LAB_0063ecad;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34,"YoYoManStageLoop");
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,"");
  local_8._0_1_ = 1;
  uStack_224 = 0x512af8;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  *param_1 = LevelYoYoMan::vftable;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[2] = 5;
  param_1[1] = 0;
  local_54 = abStack_234;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_234,"Environment");
  uVar2 = FUN_00443e30();
  param_1[5] = uVar2;
  local_58 = abStack_234;
  FUN_00511f50(abStack_234);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_00511ef0();
  FUN_00445000();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_224 = 0x512c13;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_78,PTR_DAT_006c1fd0);
  local_8._0_1_ = 5;
  local_5c = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  uStack_224 = 0x512c7a;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_98,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_7c = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
  uStack_224 = 0x512ce3;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_b8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 7;
  local_9c = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_b8);
  uStack_224 = 0x512d50;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_d8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 8;
  local_bc = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_d8);
  uStack_224 = 0x512dbd;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_f8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 9;
  local_dc = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_f8);
  uStack_224 = 0x512e29;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_118,PTR_DAT_006c1fd0);
  local_8._0_1_ = 10;
  local_fc = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_118);
  uStack_224 = 0x512e96;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_138,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xb;
  local_11c = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_138);
  uStack_224 = 0x512f03;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_158,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xc;
  local_13c = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158);
  uStack_224 = 0x512f6f;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_178,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0xd;
  local_15c = abStack_234 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_234 + -4),pcVar3);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_178);
  puVar4 = (undefined4 *)FUN_00472660();
  local_18 = *puVar4;
  local_14 = puVar4[1];
  while( true ) {
    FUN_0046df60();
    cVar1 = FUN_00426920();
    if (cVar1 == '\0') break;
    uStack_224 = 0x51301b;
    FUN_0043bc20();
    FUN_004bdc80();
    FUN_00621180();
    uStack_224 = 0x51303c;
    FUN_0042f8a0();
  }
  FUN_00512530();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

