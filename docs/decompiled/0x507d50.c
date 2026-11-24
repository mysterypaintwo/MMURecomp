// FUN_00507d50 @ 0x507d50

CAfxStringMgr * __fastcall FUN_00507d50(CAfxStringMgr *param_1)

{
  int iVar1;
  uint uVar2;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00638e0a;
  local_10 = ExceptionList;
  uVar2 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  CAfxStringMgr::CAfxStringMgr(param_1);
  local_8 = 0;
  *(undefined ***)param_1 = BaseMenuItem::vftable;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)(param_1 + 0x34));
  local_8 = CONCAT31(local_8._1_3_,1);
  FUN_0043bc00(uVar2);
  param_1[0x5c] = (CAfxStringMgr)0x1;
  FUN_00430ab0();
  *(undefined4 *)(param_1 + 0x90) = 0;
  iVar1 = DAT_006c561c;
  DAT_006c561c = DAT_006c561c + 1;
  FUN_00431160(iVar1);
  ExceptionList = local_10;
  return param_1;
}

