// FUN_00507ba0 @ 0x507ba0

void __thiscall
FUN_00507ba0(int param_1,basic_string<char,std::char_traits<char>,std::allocator<char>_> param_2)

{
  void *pvVar1;
  undefined4 in_stack_00000020;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_5c [20];
  undefined4 uStack_48;
  undefined4 local_34;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00638da1;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  if ((*(int *)(param_1 + 0x90) != 0) && (*(int *)(param_1 + 0x90) != 0)) {
    uStack_48 = 0x507bfe;
    FUN_004311b0();
  }
  uStack_48 = 0x507c16;
  FUN_00431120();
  uStack_48 = 0x507c1d;
  pvVar1 = operator_new(0x7c);
  if (pvVar1 == (void *)0x0) {
    local_34 = 0;
  }
  else {
    local_34 = FUN_00443280();
  }
  *(undefined4 *)(param_1 + 0x90) = local_34;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            (abStack_5c,
             (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &param_2);
  FUN_00443330();
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_2);
  ExceptionList = local_10;
  return;
}

