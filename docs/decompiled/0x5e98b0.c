// FUN_005e98b0 @ 0x5e98b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_005e98b0(undefined4 *param_1)

{
  char cVar1;
  undefined4 uVar2;
  char *pcVar3;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_e8 [2];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_e6 [14];
  undefined4 uStack_d8;
  char *local_a4;
  char *local_a0;
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
  puStack_c = &LAB_0063c762;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  cVar1 = FUN_004042e0();
  if (cVar1 == '\0') {
    local_a0 = "CometWomanStageLoop";
  }
  else {
    local_a0 = "CometWomanStageLoopAlt";
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34,local_a0);
  local_8 = 0;
  cVar1 = FUN_004042e0();
  if (cVar1 == '\0') {
    local_a4 = "CometWomanStageIntro";
  }
  else {
    local_a4 = "CometWomanStageIntroAlt";
  }
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50,local_a4);
  local_8._0_1_ = 1;
  uStack_d8 = 0x5e996c;
  FUN_00539770();
  local_8._0_1_ = 3;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_50);
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  *param_1 = LevelSuperCometWoman::vftable;
  param_1[0x3d] = 0;
  param_1[0x3e] = 0;
  param_1[2] = 0x1b;
  param_1[1] = 0;
  local_54 = abStack_e8;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_e8,"Environment");
  uVar2 = FUN_00443e30();
  param_1[5] = uVar2;
  local_58 = abStack_e8;
  FUN_005e93e0(abStack_e8);
  FUN_005b4970();
  FUN_005b4850();
  FUN_00445050();
  FUN_005e93d0();
  FUN_00445000();
  FUN_0043bc00();
  uStack_d8 = 0x5e9a8a;
  FUN_005e9380();
  FUN_00403ef0();
  FUN_00403ef0();
  _DAT_006c5598 = _DAT_0069899c;
  uStack_d8 = 0x5e9ac9;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_78,PTR_DAT_006c1fd0);
  local_8._0_1_ = 5;
  local_5c = abStack_e8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_e8 + -4),pcVar3);
  FUN_0053ae20();
  local_8._0_1_ = 4;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_78);
  uStack_d8 = 0x5e9b2f;
  pcVar3 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_98,PTR_DAT_006c1fd0);
  local_8._0_1_ = 6;
  local_7c = abStack_e8 + -4;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             (abStack_e8 + -4),pcVar3);
  FUN_0053ae20();
  local_8 = CONCAT31(local_8._1_3_,4);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_98);
  FUN_005391f0();
  FUN_005e9640();
  FUN_0061e440();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0061dc50();
  FUN_0061dc20();
  FUN_0061dc20();
  FUN_0053a130();
  ExceptionList = local_10;
  return param_1;
}

