// FUN_0056c110 @ 0x56c110

undefined4 * __fastcall FUN_0056c110(undefined4 *param_1)

{
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_198 [28];
  undefined1 *local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined1 *local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined1 *local_15c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *local_158;
  char *local_154;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00642637;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_154 = (char *)0x56c14c;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            (local_2c,"Enemies\\WilyMachine-Pillar\\WilyMachine-Pillar.xgun");
  local_8 = 0;
  local_154 = (char *)0x1;
  local_158 = local_2c;
  local_15c = (undefined1 *)0x56c166;
  FUN_00575f70();
  local_8 = CONCAT31(local_8._1_3_,2);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
  *param_1 = WilyMachinePillar::vftable;
  FUN_0043bc00();
  param_1[0xa3] = 0;
  param_1[0xa4] = 0;
  local_154 = (char *)0x56c1ba;
  FUN_0041f420();
  local_15c = &LAB_00425270;
  local_158 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)0x0;
  local_154 = (char *)0x0;
  local_16c = &LAB_00422af0;
  local_168 = 0;
  local_164 = 0;
  local_160 = 0;
  local_17c = &LAB_00423f10;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_198,"Wait");
  FUN_00401370();
  local_15c = &LAB_004241f0;
  local_158 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)0x0;
  local_154 = (char *)0x0;
  local_16c = &LAB_00425080;
  local_168 = 0;
  local_164 = 0;
  local_160 = 0;
  local_17c = &LAB_00427710;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_198,"Follow");
  FUN_00401370();
  local_15c = &LAB_00427700;
  local_158 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)0x0;
  local_154 = (char *)0x0;
  local_16c = &LAB_00422da0;
  local_168 = 0;
  local_164 = 0;
  local_160 = 0;
  local_17c = &LAB_00422f40;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_198,"Drop");
  FUN_00401370();
  local_15c = &LAB_00429fa0;
  local_158 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)0x0;
  local_154 = (char *)0x0;
  local_16c = &LAB_00429bb0;
  local_168 = 0;
  local_164 = 0;
  local_160 = 0;
  local_17c = &LAB_00426600;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_198,"Shoot");
  FUN_00401370();
  local_15c = &LAB_00429b70;
  local_158 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)0x0;
  local_154 = (char *)0x0;
  local_16c = &LAB_00421ba0;
  local_168 = 0;
  local_164 = 0;
  local_160 = 0;
  local_17c = &LAB_00428aa0;
  local_178 = 0;
  local_174 = 0;
  local_170 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_198,"Lift");
  FUN_00401370();
  local_154 = "Follow";
  local_158 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)0x56c6cf;
  FUN_004405c0();
  ExceptionList = local_10;
  return param_1;
}

