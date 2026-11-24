// FUN_004308c0 @ 0x4308c0

void __thiscall
FUN_004308c0(int param_1,
            basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *param_2,
            undefined1 param_3)

{
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x40),
             param_2);
  *(undefined1 *)(param_1 + 0x60) = param_3;
  return;
}

