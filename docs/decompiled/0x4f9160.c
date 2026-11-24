// FUN_004f9160 @ 0x4f9160

void __fastcall FUN_004f9160(CXMLParser *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0063e99e;
  local_10 = ExceptionList;
  uVar1 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  *(undefined ***)param_1 = PauseMenuStatus::vftable;
  local_8 = 0x1c;
  FUN_00507ca0(uVar1);
  local_8._0_1_ = 0x1b;
  _eh_vector_destructor_iterator_(param_1 + 0x3210,0x98,3,FUN_00507ca0);
  local_8._0_1_ = 0x1a;
  _eh_vector_destructor_iterator_(param_1 + 0x2fb0,0x98,4,FUN_00507ca0);
  local_8._0_1_ = 0x19;
  FUN_00507ca0();
  local_8._0_1_ = 0x18;
  FUN_00507ca0();
  local_8._0_1_ = 0x17;
  FUN_00507ca0();
  local_8._0_1_ = 0x16;
  FUN_00507ca0();
  local_8._0_1_ = 0x15;
  _eh_vector_destructor_iterator_(param_1 + 0x2af0,0x98,4,FUN_00507ca0);
  local_8._0_1_ = 0x14;
  FUN_00507ca0();
  local_8._0_1_ = 0x13;
  FUN_00507ca0();
  local_8._0_1_ = 0x12;
  FUN_00507ca0();
  local_8._0_1_ = 0x11;
  FUN_004f3b10();
  local_8._0_1_ = 0x10;
  FUN_00507ca0();
  local_8._0_1_ = 0xf;
  FUN_004f3b10();
  local_8._0_1_ = 0xe;
  FUN_00507ca0();
  local_8._0_1_ = 0xd;
  FUN_004f3b10();
  local_8._0_1_ = 0xc;
  FUN_00507ca0();
  local_8._0_1_ = 0xb;
  FUN_004f3b10();
  local_8._0_1_ = 10;
  FUN_00507ca0();
  local_8._0_1_ = 9;
  FUN_004f3b10();
  local_8._0_1_ = 8;
  FUN_00507ca0();
  local_8._0_1_ = 7;
  FUN_00624980();
  local_8._0_1_ = 6;
  FUN_00624980();
  local_8._0_1_ = 5;
  FUN_004f3b10();
  local_8._0_1_ = 4;
  _eh_vector_destructor_iterator_(param_1 + 0x18e8,0x98,0xd,FUN_00507ca0);
  local_8._0_1_ = 3;
  _eh_vector_destructor_iterator_(param_1 + 0x10c8,0xa0,0xd,FUN_00624980);
  local_8._0_1_ = 2;
  _eh_vector_destructor_iterator_(param_1 + 0x8a8,0xa0,0xd,FUN_00624980);
  local_8._0_1_ = 1;
  _eh_vector_destructor_iterator_(param_1 + 0xf0,0x98,0xd,FUN_00507ca0);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00507ca0();
  local_8 = 0xffffffff;
  CXMLParser::~CXMLParser(param_1);
  ExceptionList = local_10;
  return;
}

