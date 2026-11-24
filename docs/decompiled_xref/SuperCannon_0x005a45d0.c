// SuperCannon FUN_005a45d0 @ 0x005a45d0

undefined4 * __fastcall FUN_005a45d0(undefined4 *param_1)

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
  char *local_f4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00642275;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_f4 = (char *)0x5a460d;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"Units\\");
  local_8 = 0;
  local_114 = 0x5a462b;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_10c,
             "Enemies\\SuperCannon\\SuperCannon.xgun");
  FUN_005a6f90();
  local_8 = CONCAT31(local_8._1_3_,2);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = SuperCannon::vftable;
  FUN_0043c800();
  uVar1 = FUN_006253a0();
  param_1[0x54] = uVar1;
  fVar2 = (float10)FUN_0043c800();
  param_1[99] = (float)fVar2;
  local_fc = &LAB_004242f0;
  local_f8 = 0;
  local_f4 = (char *)0x0;
  local_10c = &LAB_0041f710;
  local_108 = 0;
  local_104 = 0;
  local_100 = 0;
  local_11c = &LAB_00426380;
  local_118 = 0;
  local_114 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_138,"Stand");
  FUN_00401370();
  local_fc = &LAB_00425040;
  local_f8 = 0;
  local_f4 = (char *)0x0;
  local_10c = &LAB_00424090;
  local_108 = 0;
  local_104 = 0;
  local_100 = 0;
  local_11c = &LAB_00427fc0;
  local_118 = 0;
  local_114 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_138,"Turn");
  FUN_00401370();
  local_fc = &LAB_00423f10;
  local_f8 = 0;
  local_f4 = (char *)0x0;
  local_10c = &LAB_00424050;
  local_108 = 0;
  local_104 = 0;
  local_100 = 0;
  local_11c = &LAB_00403eb0;
  local_118 = 0;
  local_114 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_138,"Attack");
  FUN_00401370();
  local_f4 = "Stand";
  local_f8 = 0x5a497f;
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

