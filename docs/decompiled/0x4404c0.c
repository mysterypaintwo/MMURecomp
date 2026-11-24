// FUN_004404c0 @ 0x4404c0

void __thiscall
FUN_004404c0(int param_1,basic_string<char,std::char_traits<char>,std::allocator<char>_> param_2)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int *piVar3;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_50 [20];
  undefined4 uStack_3c;
  undefined4 *puStack_38;
  undefined4 uStack_34;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_00643b29;
  local_c = ExceptionList;
  uStack_34 = DAT_006c1e94 ^ (uint)(abStack_50 + 0x20);
  ExceptionList = &local_c;
  local_4 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            (abStack_50,
             (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &param_2);
  uVar2 = FUN_004012f0();
  puVar1 = (undefined4 *)(param_1 + 0x24);
  *(undefined4 *)(param_1 + 0x2c) = uVar2;
  *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x24);
  if (*(int *)(param_1 + 0x24) != -1) {
    uStack_3c = 0x44052d;
    puStack_38 = puVar1;
    piVar3 = (int *)FUN_00433c10();
    puStack_38 = (undefined4 *)0x440560;
    (**(code **)(*piVar3 + 0x40))();
  }
  *puVar1 = uVar2;
  *(undefined4 *)(param_1 + 0x2c) = 0xffffffff;
  uStack_3c = 0x440571;
  puStack_38 = puVar1;
  piVar3 = (int *)FUN_00433c10();
  puStack_38 = (undefined4 *)0x44059a;
  (**(code **)(*piVar3 + 0x20))();
  local_4 = 0xffffffff;
  puStack_38 = (undefined4 *)0x4405a8;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_2);
  ExceptionList = local_c;
  return;
}

