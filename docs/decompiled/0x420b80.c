// FUN_00420b80 @ 0x420b80

undefined4 FUN_00420b80(char *param_1)

{
  char *pcVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_4c [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_30 [28];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00643882;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  pcVar1 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_30,param_1)
  ;
  local_8 = 0;
  this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)
         std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_4c,pcVar1);
  local_8._0_1_ = 1;
  pcVar1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
  local_14 = FUN_00401580(pcVar1);
  local_8 = (uint)local_8._1_3_ << 8;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_4c);
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_30);
  ExceptionList = local_10;
  return local_14;
}

