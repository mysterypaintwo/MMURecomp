// FUN_00402850 @ 0x402850

basic_string<char,std::char_traits<char>,std::allocator<char>_> * __thiscall
FUN_00402850(basic_string<char,std::char_traits<char>,std::allocator<char>_> *param_1,
            basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *param_2)

{
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=(param_1,param_2);
  *(undefined4 *)(param_1 + 0x1c) = *(undefined4 *)(param_2 + 0x1c);
  return param_1;
}

