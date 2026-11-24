// FUN_00614d30 @ 0x614d30

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00614d30(int param_1)

{
  bool bVar1;
  char cVar2;
  uint uVar3;
  undefined4 uVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  int iVar5;
  basic_ostream<char,struct_std::char_traits<char>_> *pbVar6;
  basic_ostream<char,std::char_traits<char>_> *pbVar7;
  undefined1 *puVar8;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_1c8 [36];
  undefined1 local_1a4 [16];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_194 [36];
  undefined1 local_170 [24];
  undefined1 local_158 [32];
  undefined1 local_138 [16];
  basic_stringstream<char,std::char_traits<char>,std::allocator<char>_> local_128 [8];
  basic_ostream<char,std::char_traits<char>_> local_120 [132];
  int local_9c;
  basic_stringstream<char,std::char_traits<char>,std::allocator<char>_> local_98 [8];
  basic_ostream<char,std::char_traits<char>_> local_90 [128];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_006376d1;
  local_10 = ExceptionList;
  uVar3 = DAT_006c1e94 ^ (uint)(local_90 + 0x8c);
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0xd2c) = _DAT_006987b0;
  FUN_00623fc0(param_1 + 0xf0);
  uVar4 = FUN_0043bc20(0,0);
  puVar8 = local_138;
  FUN_00401930(puVar8,uVar4,uVar3);
  uVar4 = FUN_004014c0(puVar8,uVar4);
  FUN_004026e0(uVar4);
  FUN_00623fc0(param_1 + 0x220);
  uVar4 = FUN_0043bc20(0x30,0x10);
  FUN_004026e0(uVar4);
  FUN_00623fc0(param_1 + 0x188);
  uVar4 = FUN_0043bc20(0x40,0x80);
  FUN_004026e0(uVar4);
  uVar4 = 0;
  FUN_00403240(0);
  FUN_00403220(uVar4);
  FUN_00623fc0(param_1 + 0x58);
  uVar4 = FUN_0043bc20(0x28,0x28);
  puVar8 = local_158;
  FUN_00401930(puVar8,uVar4);
  uVar4 = FUN_004014c0(puVar8,uVar4);
  FUN_004026e0(uVar4);
  for (local_9c = 0; local_9c < 10; local_9c = local_9c + 1) {
    this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_00402700();
    bVar1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::empty(this);
    if (!bVar1) {
      FUN_00623fc0(param_1 + 0x5b8 + local_9c * 0xb8);
      uVar4 = FUN_0043bc20(0x38,local_9c * 0x10 + 0x28);
      FUN_004026e0(uVar4);
    }
  }
  FUN_00441b70(0);
  FUN_00623fc0(param_1 + 0x2d8);
  uVar4 = FUN_0043bc20(0x40,0x38);
  FUN_004026e0(uVar4);
  FUN_00623fc0(param_1 + 0x390);
  std::basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>::
  basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>(local_98,3);
  local_8 = 0;
  iVar5 = FUN_004034b0();
  uVar4 = std::setw((int)local_170);
  pbVar6 = std::basic_ostream<char,std::char_traits<char>_>::operator<<(local_90,FUN_004032a0);
  pbVar7 = (basic_ostream<char,std::char_traits<char>_> *)FUN_00403450(pbVar6,uVar4);
  std::basic_ostream<char,std::char_traits<char>_>::operator<<(pbVar7,iVar5);
  uVar4 = FUN_0043bc20(0x90,0x38);
  FUN_004026e0(uVar4);
  uVar4 = std::basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>::str(local_98);
  local_8._0_1_ = 1;
  FUN_004f3ab0(uVar4);
  local_8 = (uint)local_8._1_3_ << 8;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_194);
  cVar2 = FUN_004033e0();
  if (cVar2 == '\0') {
    cVar2 = FUN_00403400();
    if (cVar2 == '\0') goto LAB_006151cd;
  }
  FUN_00623fc0(param_1 + 0x448);
  uVar4 = FUN_0043bc20(0x40,0x68);
  FUN_004026e0(uVar4);
  FUN_00623fc0(param_1 + 0x500);
  std::basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>::
  basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>(local_128,3);
  local_8._0_1_ = 2;
  iVar5 = FUN_004034d0();
  uVar4 = std::setw((int)local_1a4);
  pbVar6 = std::basic_ostream<char,std::char_traits<char>_>::operator<<(local_120,FUN_004032a0);
  pbVar7 = (basic_ostream<char,std::char_traits<char>_> *)FUN_00403450(pbVar6,uVar4);
  std::basic_ostream<char,std::char_traits<char>_>::operator<<(pbVar7,iVar5);
  uVar4 = FUN_0043bc20(0x90,0x68);
  FUN_004026e0(uVar4);
  uVar4 = std::basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>::str(local_128)
  ;
  local_8._0_1_ = 3;
  FUN_004f3ab0(uVar4);
  local_8._0_1_ = 2;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_1c8);
  local_8 = (uint)local_8._1_3_ << 8;
  std::basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>::_vbase_destructor_
            (local_128);
LAB_006151cd:
  local_8 = 0xffffffff;
  std::basic_stringstream<char,std::char_traits<char>,std::allocator<char>_>::_vbase_destructor_
            (local_98);
  ExceptionList = local_10;
  return;
}

