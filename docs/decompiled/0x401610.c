// FUN_00401610 @ 0x401610

void __thiscall
FUN_00401610(int param_1,basic_string<char,std::char_traits<char>,std::allocator<char>_> param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00642d29;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x34),
             (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &param_2);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_2);
  ExceptionList = local_10;
  return;
}

