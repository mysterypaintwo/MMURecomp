// FUN_005081a0 @ 0x5081a0

undefined * FUN_005081a0(void)

{
  bool bVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  undefined *puVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0063708c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((DAT_006c7110 & 1) == 0) {
    DAT_006c7110 = DAT_006c7110 | 1;
    local_8 = 0;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"SYSTEM MENU");
    local_8._0_1_ = 1;
    FUN_00402720(10,local_2c);
    local_8 = (uint)local_8._1_3_ << 8;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
    _atexit(FUN_00647420);
  }
  local_8 = 0xffffffff;
  this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00402b30(0);
  bVar1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::empty(this);
  if (bVar1) {
    puVar2 = &DAT_006c6ad4;
    FUN_00402b30(0);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c6a1c;
    FUN_00402b30(1);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c6a58;
    FUN_00402b30(2);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c6924;
    FUN_00402b30(3);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c68c4;
    FUN_00402b30(4);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c68c4;
    FUN_00402b30(5);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c68c4;
    FUN_00402b30(6);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c68c4;
    FUN_00402b30(7);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c68c4;
    FUN_00402b30(8);
    FUN_00402850(puVar2);
  }
  ExceptionList = local_10;
  return &DAT_006c70dc;
}

