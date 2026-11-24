// FUN_00401370 @ 0x401370

void FUN_00401370(basic_string<char,std::char_traits<char>,std::allocator<char>_> param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  undefined4 in_stack_00000020;
  undefined4 in_stack_00000024;
  undefined4 in_stack_00000028;
  undefined4 in_stack_0000002c;
  undefined4 in_stack_00000030;
  undefined4 in_stack_00000034;
  undefined4 in_stack_00000038;
  undefined4 in_stack_0000003c;
  undefined4 in_stack_00000040;
  undefined4 in_stack_00000044;
  undefined4 in_stack_00000048;
  undefined4 in_stack_0000004c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_58 [20];
  undefined4 uStack_44;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *local_30;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_006438e4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  uStack_44 = 0x4013a6;
  pvVar1 = operator_new(0x50);
  local_8._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    local_30 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)0x0;
  }
  else {
    local_30 = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_004014a0();
  }
  local_8 = (uint)local_8._1_3_ << 8;
  *(undefined4 *)(local_30 + 0x20) = in_stack_00000020;
  *(undefined4 *)(local_30 + 0x24) = in_stack_00000024;
  *(undefined4 *)(local_30 + 0x28) = in_stack_00000028;
  *(undefined4 *)(local_30 + 0x2c) = in_stack_0000002c;
  *(undefined4 *)(local_30 + 0x30) = in_stack_00000030;
  *(undefined4 *)(local_30 + 0x34) = in_stack_00000034;
  *(undefined4 *)(local_30 + 0x38) = in_stack_00000038;
  *(undefined4 *)(local_30 + 0x3c) = in_stack_0000003c;
  *(undefined4 *)(local_30 + 0x40) = in_stack_00000040;
  *(undefined4 *)(local_30 + 0x44) = in_stack_00000044;
  *(undefined4 *)(local_30 + 0x48) = in_stack_00000048;
  *(undefined4 *)(local_30 + 0x4c) = in_stack_0000004c;
  uStack_44 = 0x40143e;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
            (local_30,(basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_>
                       *)&param_1);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            (abStack_58,
             (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &param_1);
  FUN_004012f0();
  uStack_44 = 0x401473;
  puVar2 = (undefined4 *)FUN_00433c10();
  *puVar2 = local_30;
  local_8 = 0xffffffff;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_1);
  ExceptionList = local_10;
  return;
}

