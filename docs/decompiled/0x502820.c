// FUN_00502820 @ 0x502820

undefined4 * __fastcall FUN_00502820(undefined4 *param_1)

{
  char cVar1;
  char *pcVar2;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *pbVar3;
  bool bVar4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_264 [28];
  undefined1 *local_248;
  undefined4 local_244;
  undefined4 local_234;
  undefined4 local_230;
  undefined4 local_22c;
  undefined1 *local_228;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *local_1c4;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_174 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_158 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_13c [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_120 [28];
  undefined1 *local_104;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_100 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_e4 [28];
  undefined1 *local_c8;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_c4 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_a8 [28];
  undefined1 *local_8c;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_88 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_6c [28];
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> local_50 [28];
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_34 [28];
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *local_18;
  undefined1 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0063a779;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00624140();
  local_8 = 0;
  *param_1 = MenuInGame::vftable;
  param_1[0x16] = 0;
  param_1[0x17] = 0;
  param_1[0x18] = 0;
  *(undefined1 *)(param_1 + 0x19) = 0;
  *(undefined1 *)((int)param_1 + 0x65) = 0;
  *(undefined1 *)((int)param_1 + 0x66) = 0;
  *(undefined1 *)((int)param_1 + 0x67) = 0;
  *(undefined1 *)(param_1 + 0x1a) = 0;
  *(undefined1 *)((int)param_1 + 0x69) = 0;
  FUN_004f3b80();
  local_8._0_1_ = 1;
  FUN_004f3b80();
  local_8._0_1_ = 2;
  FUN_004f3b80();
  local_8._0_1_ = 3;
  FUN_004f3b80();
  local_8._0_1_ = 4;
  FUN_004f3b80();
  local_8._0_1_ = 5;
  FUN_004f3b80();
  local_8._0_1_ = 6;
  FUN_004f3b80();
  local_8._0_1_ = 7;
  FUN_004f3b80();
  local_8._0_1_ = 8;
  FUN_004f3b80();
  local_8._0_1_ = 9;
  FUN_00443280();
  local_8._0_1_ = 10;
  FUN_00443280();
  local_8._0_1_ = 0xb;
  FUN_00443280();
  local_8._0_1_ = 0xc;
  FUN_00443280();
  local_8._0_1_ = 0xd;
  cVar1 = FUN_00403d40();
  bVar4 = cVar1 == '\0';
  if (bVar4) {
    pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                     basic_string<char,std::char_traits<char>,std::allocator<char>_>
                               (local_6c,PTR_DAT_006c1fd0);
    local_8 = 0x10;
    local_228 = (undefined1 *)0x502ada;
    local_1c4 = (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_88,pcVar2);
    local_8 = 0x11;
  }
  else {
    pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                     basic_string<char,std::char_traits<char>,std::allocator<char>_>
                               (local_34,PTR_DAT_006c1fd0);
    local_8 = CONCAT31(local_8._1_3_,0xe);
    local_228 = (undefined1 *)0x502a4d;
    local_1c4 = (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
                std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_50,pcVar2);
    local_8 = 0xf;
  }
  local_18 = local_1c4;
  local_14 = (undefined1 *)&local_234;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)&local_234,local_1c4
            );
  FUN_00443330();
  local_8 = 0x10;
  if (bVar4) {
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
              ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_88);
  }
  local_8 = 0xf;
  if (bVar4) {
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_6c);
  }
  local_8 = 0xe;
  if (!bVar4) {
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
              ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_50);
  }
  local_8 = 0xd;
  if (!bVar4) {
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_34);
  }
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_a8,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x12;
  local_228 = (undefined1 *)0x502c10;
  pbVar3 = (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
           std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_c4,pcVar2);
  local_8._0_1_ = 0x13;
  local_8c = (undefined1 *)&local_234;
  local_244 = 0x502c47;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &local_234,pbVar3);
  FUN_00443330();
  local_8._0_1_ = 0x12;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_c4);
  local_8._0_1_ = 0xd;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_a8);
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_e4,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x14;
  local_228 = (undefined1 *)0x502cc3;
  pbVar3 = (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
           std::operator+<char,std::char_traits<char>,std::allocator<char>_>(local_100,pcVar2);
  local_8._0_1_ = 0x15;
  local_c8 = (undefined1 *)&local_234;
  local_244 = 0x502cfa;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &local_234,pbVar3);
  FUN_00443330();
  local_8._0_1_ = 0x14;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,std::char_traits<char>,std::allocator<char>_> *)local_100);
  local_8._0_1_ = 0xd;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_e4);
  pcVar2 = (char *)std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (local_120,PTR_DAT_006c1fd0);
  local_8._0_1_ = 0x16;
  local_104 = (undefined1 *)&local_234;
  local_244 = 0x502d7a;
  std::operator+<char,std::char_traits<char>,std::allocator<char>_>
            ((basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
             &local_234,pcVar2);
  FUN_00443330();
  local_8._0_1_ = 0xd;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_120);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_13c,"SCREEN");
  local_8._0_1_ = 0x17;
  FUN_004f3ab0();
  local_8._0_1_ = 0xd;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_13c);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158,"BOSS");
  local_8._0_1_ = 0x18;
  FUN_004f3ab0();
  local_8._0_1_ = 0xd;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_158);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_174,"STAGE");
  local_8._0_1_ = 0x19;
  FUN_004f3ab0();
  local_8 = CONCAT31(local_8._1_3_,0xd);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(local_174);
  local_228 = &LAB_00403290;
  local_234 = 0;
  local_230 = 0;
  local_22c = 0;
  local_248 = &LAB_00433070;
  local_244 = 0;
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
  basic_string<char,std::char_traits<char>,std::allocator<char>_>(abStack_264,"InGame");
  FUN_00401370();
  FUN_004405c0();
  FUN_00431120();
  FUN_00431120();
  FUN_00431120();
  FUN_00431120();
  FUN_00431120();
  ExceptionList = local_10;
  return param_1;
}

