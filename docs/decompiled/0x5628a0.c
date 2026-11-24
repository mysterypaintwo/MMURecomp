// FUN_005628a0 @ 0x5628a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * __thiscall
FUN_005628a0(int param_1,undefined1 param_2,undefined4 param_3,
            basic_string<char,std::char_traits<char>,std::allocator<char>_> param_4)

{
  undefined4 uStack00000028;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_54 [16];
  undefined4 uStack_44;
  int **ppiStack_40;
  undefined4 *puStack_3c;
  uint uStack_38;
  undefined1 local_31;
  int *local_30;
  undefined4 local_2c;
  int local_28;
  int *local_24;
  undefined1 *local_20;
  int *local_1c;
  int *local_18 [2];
  void *local_10;
  undefined1 *puStack_c;
  int *local_8;
  
  puStack_c = &LAB_00637bec;
  local_10 = ExceptionList;
  uStack_38 = DAT_006c1e94 ^ (uint)(local_18 + 5);
  ExceptionList = &local_10;
  local_8 = (int *)0x0;
  local_28 = param_1;
  if (*(char *)(param_1 + 0x29) == '\0') {
    local_1c = (int *)0x0;
    local_8 = (int *)0xffffffff;
    puStack_3c = (undefined4 *)0x5628f1;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_4);
  }
  else {
    local_20 = abStack_54;
    local_2c = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
               basic_string<char,std::char_traits<char>,std::allocator<char>_>
                         (abStack_54,
                          (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_>
                           *)&param_4);
    local_30 = (int *)FUN_00443e30();
    puStack_3c = &uStack00000028;
    ppiStack_40 = (int **)0x562928;
    local_18[0] = local_30;
    FUN_00426870();
    local_31 = (float)_DAT_00697ce0 <= uStack00000028;
    puStack_3c = (undefined4 *)(uint)(byte)local_31;
    ppiStack_40 = (int **)0x562954;
    (**(code **)(*local_18[0] + 0x14))();
    ppiStack_40 = (int **)&param_2;
    uStack_44 = 0x562960;
    FUN_00403ef0();
    ppiStack_40 = (int **)local_28;
    uStack_44 = 0x56296c;
    FUN_004268b0();
    ppiStack_40 = local_18;
    uStack_44 = 0x56297b;
    FUN_00433600();
    local_24 = local_18[0];
    local_8 = (int *)0xffffffff;
    ppiStack_40 = (int **)0x562991;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
    ~basic_string<char,std::char_traits<char>,std::allocator<char>_>(&param_4);
    local_1c = local_24;
  }
  ExceptionList = local_10;
  return local_1c;
}

