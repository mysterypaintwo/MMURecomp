// SkateBoardMan FUN_005a5210 @ 0x005a5210

undefined4 * __fastcall FUN_005a5210(undefined4 *param_1)

{
  undefined4 uVar1;
  float10 fVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_138 [28];
  undefined1 *local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined1 *local_10c;
  undefined4 local_108;
  undefined4 local_104;
  undefined4 local_100;
  undefined1 *local_fc;
  undefined4 local_f8;
  undefined *local_f4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_006422a7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_f4 = (undefined *)0x5a524d;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_114 = 0x5a526b;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_10c,
             "Enemies\\SkateBoardMan\\SkateBoardMan.xgun");
  FUN_005a6f90();
  local_8 = CONCAT31(local_8._1_3_,2);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = SkateBoardMan::vftable;
  *(undefined1 *)(param_1 + 0x65) = 1;
  FUN_0043c800();
  uVar1 = FUN_006253a0();
  param_1[0x54] = uVar1;
  if (DAT_006c55f0 != 0) {
    uVar1 = FUN_00462a90();
    param_1[99] = uVar1;
  }
  fVar2 = (float10)FUN_0043c800();
  param_1[0x18] = (float)fVar2;
  param_1[0x19] = 0;
  fVar2 = (float10)FUN_0043c800();
  param_1[100] = (float)fVar2;
  fVar2 = (float10)FUN_0043c800();
  param_1[0x66] = (float)fVar2;
  fVar2 = (float10)FUN_0043c800();
  param_1[0x67] = (float)fVar2;
  fVar2 = (float10)FUN_0043c800();
  param_1[0x68] = (float)fVar2;
  local_fc = &LAB_004242f0;
  local_f8 = 0;
  local_f4 = (undefined *)0x0;
  local_10c = &LAB_0041f710;
  local_108 = 0;
  local_104 = 0;
  local_100 = 0;
  local_11c = &LAB_00426380;
  local_118 = 0;
  local_114 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_138,"Jump");
  FUN_00401370();
  local_fc = &LAB_00425040;
  local_f8 = 0;
  local_f4 = (undefined *)0x0;
  local_10c = &LAB_00424090;
  local_108 = 0;
  local_104 = 0;
  local_100 = 0;
  local_11c = &LAB_00427fc0;
  local_118 = 0;
  local_114 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_138,"TurnAround");
  FUN_00401370();
  local_fc = &LAB_00423f10;
  local_f8 = 0;
  local_f4 = (undefined *)0x0;
  local_10c = &LAB_00424050;
  local_108 = 0;
  local_104 = 0;
  local_100 = 0;
  local_11c = &LAB_00403eb0;
  local_118 = 0;
  local_114 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_138,"Move");
  FUN_00401370();
  local_f4 = &DAT_0067d2e0;
  local_f8 = 0x5a5664;
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

