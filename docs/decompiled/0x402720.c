// FUN_00402720 @ 0x402720

basic_string<char,std::char_traits<char>,std::allocator<char>_> * __thiscall
FUN_00402720(basic_string<char,std::char_traits<char>,std::allocator<char>_> *param_1,
            undefined4 param_2,
            basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *param_3)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00643579;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(param_1,param_3);
  local_8 = 0;
  FUN_00402920(param_2);
  ExceptionList = local_10;
  return param_1;
}

