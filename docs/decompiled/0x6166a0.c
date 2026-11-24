// FUN_006166a0 @ 0x6166a0

undefined * FUN_006166a0(void)

{
  bool bVar1;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  undefined *puVar2;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_2c [28];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_006377b8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((DAT_006c7538 & 1) == 0) {
    DAT_006c7538 = DAT_006c7538 | 1;
    local_8 = 0;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c,"ENDLESS ATTACK");
    local_8._0_1_ = 1;
    FUN_00402720(10,local_2c);
    local_8 = (uint)local_8._1_3_ << 8;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_2c);
    _atexit(FUN_00646cd0);
  }
  local_8 = 0xffffffff;
  this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00402b30(0);
  bVar1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::empty(this);
  if (bVar1) {
    puVar2 = &DAT_006c588c;
    FUN_00402b30(0);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c584c;
    FUN_00402b30(1);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c584c;
    FUN_00402b30(2);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c58ec;
    FUN_00402b30(3);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c584c;
    FUN_00402b30(4);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c584c;
    FUN_00402b30(5);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c584c;
    FUN_00402b30(6);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c58ac;
    FUN_00402b30(7);
    FUN_00402850(puVar2);
    puVar2 = &DAT_006c586c;
    FUN_00402b30(8);
    FUN_00402850(puVar2);
  }
  ExceptionList = local_10;
  return &DAT_006c7504;
}

